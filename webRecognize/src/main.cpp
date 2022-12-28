
#include <spdlog/fmt/bundled/format.h>

// nolitedb
#include "Controller/WebsocketVideoBufferController.hpp"
#include "DAL/NoLiteDB/NotificationRepositoryNLDB.hpp"
#include "DAL/NoLiteDB/VideoBufferRepositoryNLDB.hpp"
#include "LiveVideo/LiveViewExceptions.hpp"
#include "Pattern/VideoBufferSubscriberPublisher.hpp"
#include "VideoBufferTasksManager.hpp"
#include "nldb/LOG/managers/log_constants.hpp"
#include "nldb/SQL3Implementation.hpp"

//
#include "Controller/NotificationController.hpp"
#include "DAL/ConfigurationDAO.hpp"
#include "LiveVideo/CameraLiveVideo.hpp"
#include "LiveVideo/LiveVideo.hpp"
#include "LiveVideo/ObserverLiveVideo.hpp"
#include "Serialization/JsonAvailableConfigurationDTO.hpp"
#include "Serialization/JsonSerialization.hpp"
#include "Utils/StringUtils.hpp"
#include "Utils/VideoBuffer.hpp"
#include "nldb/backends/sqlite3/DB/DB.hpp"
#include "observer/Blob/BlobDetector/Blob.hpp"
#include "observer/Blob/BlobDetector/Finding.hpp"
#include "observer/Domain/Configuration/CameraConfiguration.hpp"
#include "observer/Domain/Configuration/ConfigurationParser.hpp"
#include "observer/Domain/ObserverCentral.hpp"
#include "observer/Log/log.hpp"
#include "observer/Size.hpp"
#include "server_types.hpp"
#include "server_utils.hpp"
#include "stream_content/FileExtension.hpp"
#include "stream_content/FileReader.hpp"
#include "stream_content/FileStreamer.hpp"
#include "uWebSockets/App.h"
#include "uWebSockets/HttpContextData.h"
#include "uWebSockets/Multipart.h"

// Selects a Region of interes from a camera fram
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string_view>
#include <unordered_map>

// #include "AreaSelector.hpp"
#include "Server/ServerContext.hpp"
#include "SocketData.hpp"
#include "base64.hpp"

// DAL
#include "DAL/InMemory/CameraRepository.hpp"
#include "DAL/InMemory/NotificationRepository.hpp"

// CL
#include "CL/NotificationCL.hpp"
#include "Utils/JsonUtils.hpp"

const bool SSL = false;
namespace fs = std::filesystem;
namespace rc = Observer;
typedef cv::Mat TFrame;

static const std::string liveViewPrefix = "/live/";
static const std::string liveViewWsRoute = liveViewPrefix + "*";

constexpr int OBSERVER_LIVE_VIEW_MAX_FPS = 20;

int main() {
    /**
     * Basic guidelines/recommendation
     *   - Don't take too long to send the response
     *
     *   - If the resource at /api/ is not found send the 404 but make sure to
     *     set the cache control at a few seconds so the browser knows that the
     *     resource is the one that doesn't exist, not the uri.
     *     Note: some browser cache it and some don't.
     *
     *   - uWebsocket matches every /, so if you listen to /api/endpoint/ and
     *     then you make a request to /api/endpoint it won't work but
     *     /api/endpoint/ will work.
     */

    // initialize logger
    Observer::LogManager::Initialize();
    nldb::LogManager::Initialize();
    nldb::LogManager::SetLevel(nldb::log_level::warn);

    // recognizer instance
    std::shared_ptr<rc::ObserverCentral> recognizer;

    std::vector<IFunctionality*> threads;

    /* ----------------- CONFIGURATIONS DAO ----------------- */
    nldb::DBSL3 configurationsDB;
    if (!configurationsDB.open("configurations.db")) {
        OBSERVER_ERROR("Could not open the database!");
        configurationsDB.throwLastError();
    }

    Web::DAL::ConfigurationDAO configurationDAO(&configurationsDB);

    /* ---------------- MAIN UWEBSOCKETS APP ---------------- */
    auto app = uWS::App();

    Web::ServerContext<SSL> serverCtx = {
        .rootFolder = fs::current_path(),
        .port = 3001,
        .recognizeContext = {false, nullptr},

        // this should not be like this, allocating on heap. This should be on a
        // class that manages all of this, like App or Server. But for now it
        // will stay like this until more features are added and become stable.

        .liveViewsManager = std::make_unique<Web::LiveViewsManager<SSL>>(
            OBSERVER_LIVE_VIEW_MAX_FPS, &serverCtx.recognizeContext)};

    FileStreamer::Init<SSL>(serverCtx.rootFolder);

    /* -------------- NOTIFICATIONS REPOSITORY -------------- */
    nldb::DBSL3 notificationsDB;
    if (!notificationsDB.open("notifications.db")) {
        OBSERVER_ERROR("Could not open the database!");
        notificationsDB.throwLastError();
    }

    Web::DAL::NotificationRepositoryNLDB notificationRepository(
        &notificationsDB);
    Web::CL::NotificationCL notificationCache(&notificationRepository);
    Web::Controller::NotificationController<SSL> notificationController(
        &app, &notificationRepository, &notificationCache, &configurationDAO,
        &serverCtx);

    /* ----------------- CAMERAS REPOSITORY ----------------- */
    Web::DAL::CameraRepositoryMemory cameraRepository;

    // for (auto&& cam : cfg.cameras) {
    //     auto camera = Web::Domain::Camera(cam.name, cam.url);
    //     cameraRepository.Add(camera);
    // }

    /* ---------------- CACHED CAMERA IMAGES ---------------- */
    // url -> image
    std::unordered_map<std::string, std::vector<unsigned char>>
        cachedCameraImage;

    /* -------------------- VIDEO BUFFER -------------------- */
    // TODO: Move to controller
    // video buffer stores path to video buffer with the fps that it was
    // captured. Used to easily modify the camera's blob parameters.
    nldb::DBSL3 videoBufferDB;
    if (!videoBufferDB.open("video_buffer.db")) {
        OBSERVER_ERROR("Could not open the database!");
        videoBufferDB.throwLastError();
    }

    Web::WebsocketVideoBufferController<SSL> bufferWebSocket;
    Web::DAL::VideoBufferRepositoryNLDB videoBufferRepository(&videoBufferDB);
    Web::VideoBufferTasksManager videoBufferTasksManager(&videoBufferRepository,
                                                         &configurationDAO);

    videoBufferTasksManager.SubscribeToTaskResult(&bufferWebSocket);

    videoBufferTasksManager.Start();
    bufferWebSocket.Start();

    threads.push_back(&videoBufferTasksManager);
    threads.push_back(&bufferWebSocket);

    /* ------------------- CREATE OBSERVER ------------------ */
    const auto startRecognize = [&observerCtx = serverCtx.recognizeContext,
                                 &notificationController,
                                 &notificationRepository,
                                 &threads](const Observer::Configuration& cfg) {
        if (observerCtx.observer)
            OBSERVER_WARN("Observer wasn't null at start");

        observerCtx.observer = std::make_unique<Observer::ObserverCentral>(
            cfg, notificationRepository.GetLastGroupID());

        observerCtx.observer->SubscribeToNewNotifications(
            (Observer::INotificationEventSubscriber*)&notificationController);

        observerCtx.observer->Start();

        observerCtx.running = true;
    };

    const auto stopRecognize = [&observerCtx = serverCtx.recognizeContext]() {
        if (observerCtx.observer && observerCtx.running) {
            observerCtx.observer->Stop();
        }

        observerCtx.running = false;
        observerCtx.observer = nullptr;
    };

    /* ----------------- LISTEN TO REQUESTS ----------------- */
    app.listen(serverCtx.port,
               [&serverCtx](auto* token) {
                   if (token) {
                       OBSERVER_INFO(
                           "Serving folder {0} over HTTP at "
                           "http://localhost:{1}",
                           serverCtx.rootFolder, serverCtx.port);
                   }
               })

        .put(
            "/api/buffer/",
            [&configurationDAO, &videoBufferRepository,
             &videoBufferTasksManager](auto* res, auto* req) {
                /**
                 * {
                 *      duration <double>: buffer duration in seconds
                 *
                 *      delay <double>: seconds to wait before starting the
                 *
                 *      resize <{width, height}>:
                 *          size to which resize the frames, set to 0,0 to use
                 *          the camera resolution
                 *
                 *      camera_id <string>:
                 *          Will use this camera configuration url to get the
                 *          buffer
                 * }
                 *
                 */
                std::string buffer;

                // get it before we attach the reader
                auto ct = std::string(req->getHeader("content-type"));

                res->onAborted([]() {});

                res->onData([&configurationDAO, &videoBufferRepository,
                             &videoBufferTasksManager, res, req, ct,
                             buffer = std::move(buffer)](std::string_view data,
                                                         bool last) mutable {
                    buffer.append(data.data(), data.length());

                    if (last) {
                        if (ct != "application/json") {
                            res->writeStatus(HTTP_400_BAD_REQUEST)
                                ->end("Expected a json body");
                            return;
                        }

                        nldb::json bufferAsJson;
                        try {
                            bufferAsJson = nlohmann::json::parse(buffer);
                        } catch (const std::exception& e) {
                            res->writeStatus(HTTP_400_BAD_REQUEST)
                                ->endProblemJson(
                                    (nlohmann::json {
                                         {"title", "body isn't a valid json"}})
                                        .dump());
                            return;
                        }

                        // TODO: this->ensureJson(duration, Type::Number)

                        double duration;

                        try {
                            duration = bufferAsJson["duration"].get<double>();
                        } catch (const std::exception& e) {
                            res->writeStatus(HTTP_400_BAD_REQUEST)
                                ->endProblemJson(
                                    (nlohmann::json {
                                         {"title",
                                          "required json "
                                          "numeric field 'duration'"}})
                                        .dump());
                            return;
                        }

                        double delay;

                        try {
                            delay = bufferAsJson["delay"].get<double>();
                        } catch (const std::exception& e) {
                            res->writeStatus(HTTP_400_BAD_REQUEST)
                                ->endProblemJson(
                                    (nlohmann::json {{"title",
                                                      "required json "
                                                      "numeric field 'delay'"}})
                                        .dump());
                            return;
                        }

                        Observer::Size resize;

                        try {
                            resize =
                                bufferAsJson["resize"].get<Observer::Size>();
                        } catch (const std::exception& e) {
                            res->writeStatus(HTTP_400_BAD_REQUEST)
                                ->endProblemJson(
                                    (nlohmann::json {{"title",
                                                      "required json "
                                                      "field 'resize' of type "
                                                      "{width, height}"}})
                                        .dump());
                            return;
                        }

                        std::string cameraID;
                        try {
                            cameraID =
                                bufferAsJson["camera_id"].get<std::string>();
                        } catch (const std::exception& e) {
                            res->writeStatus(HTTP_400_BAD_REQUEST)
                                ->endProblemJson(
                                    (nlohmann::json {
                                         {"title",
                                          "required json "
                                          "field 'camera_id' of type string"}})
                                        .dump());
                            return;
                        }

                        std::string cameraUri;
                        try {
                            nldb::json camera =
                                configurationDAO.GetCamera(cameraID);
                            cameraUri = camera["url"];
                        } catch (const std::exception& e) {
                            res->writeStatus(HTTP_404_NOT_FOUND)
                                ->writeHeader("Cache-Control", "max-age=5")
                                ->endProblemJson(
                                    (nlohmann::json {
                                         {"title",
                                          "camera with the specified id "
                                          "doesn't exists"}})
                                        .dump());
                            return;
                        }

                        nlohmann::json initial = {{"camera_id", cameraID},
                                                  {"duration", duration},
                                                  {"date_unix", time(0)},
                                                  {"state", "without_buffer"}};

                        // instead of using random number I prefer to use the
                        // db id, add a new object with the fps to get the an id
                        std::string id = videoBufferRepository.Add(initial);

                        // async -> create the buffer task
                        videoBufferTasksManager.AddTask(
                            Web::GenerateVideoBufferTask {
                                .videoBufferTaskID = id,
                                .cameraID = cameraID,
                                .uri = cameraUri,
                                .duration = duration,
                                .delay = delay,
                                .resizeTo = resize});

                        initial["id"] = id;

                        res->writeStatus(HTTP_202_Accepted)
                            ->endJson(initial.dump());
                    }
                });
            })

        .get("/api/buffer/",
             [&videoBufferRepository](auto* res, auto* req) {
                 std::string cameraID(req->getQuery("camera_id"));

                 /**
                  * state:
                  *     - without_buffer: has id, camera_id, date_unix and
                  *       duration
                  *     - with_buffer: has previous and fps
                  *     - detected: has previous and contours, blobs
                  */
                 res->endJson((cameraID.empty()
                                   ? videoBufferRepository.GetAll()
                                   : videoBufferRepository.GetAll(cameraID))
                                  .dump());
             })

        .del("/api/buffer/:id",
             [&videoBufferRepository](auto* res, auto* req) {
                 std::string id(req->getParameter(0));

                 if (auto buffer = videoBufferRepository.GetInternal(id)) {
                     if (buffer->contains("path")) {
                         const std::string cameraFramesPath =
                             buffer->at("path");
                         remove(cameraFramesPath.c_str());
                     }

                     if (buffer->contains("diffFramesPath")) {
                         const std::string diffFramesPath =
                             buffer->at("diffFramesPath");
                         remove(diffFramesPath.c_str());
                     }

                     videoBufferRepository.Remove(id);

                     res->end();
                 } else {
                     res->writeStatus(HTTP_404_NOT_FOUND)
                         ->writeHeader("Cache-Control", "max-age=10")
                         ->endProblemJson(
                             (nldb::json {{"title", "buffer not found"}})
                                 .dump());
                     return;
                 }
             })

        .get("/stream/buffer/:id",
             [&videoBufferRepository](auto* res, auto* req) {
                 std::string id(req->getParameter(0));
                 std::string rangeHeader(req->getHeader("range"));

                 // type = raw | diff
                 std::string streamType(req->getQuery("type"));

                 if (videoBufferRepository.Exists(id)) {
                     std::optional<std::string> filename;

                     if (streamType == "raw") {
                         filename = videoBufferRepository.GetRawBufferPath(id);
                     } else {
                         filename = videoBufferRepository.GetDiffBufferPath(id);
                     }

                     if (!filename.has_value()) {
                         res->writeStatus(HTTP_400_BAD_REQUEST)
                             ->endProblemJson(
                                 (nldb::json {{"title",
                                               "buffer still doesn't have the "
                                               "frames requested"}})
                                     .dump());
                         return;
                     }

                     FileStreamer::GetInstance().streamFile(
                         res, filename.value(), rangeHeader);
                 } else {
                     res->writeStatus(HTTP_404_NOT_FOUND);
                     res->end();
                 }
             })

        /**
         * @brief Request a live view of the camera. If successfully
         * generated the live view it returns a json with the ws_feed_path,
         * to wich the client can request a WebSocket connection and we will
         * provide the images through it
         */
        .get("/api/requestCameraStream",
             [&serverCtx](auto* res, auto* req) {
                 std::string uri(req->getQuery("uri"));

                 if (serverCtx.liveViewsManager->CreateCameraView(uri)) {
                     std::string feed_id;

                     try {
                         feed_id = serverCtx.liveViewsManager->GetFeedId(uri);
                     } catch (const Web::InvalidCameraUriException& e) {
                         nlohmann::json error = {
                             {"title", "Invalid camera uri"}};

                         res->writeStatus(HTTP_404_NOT_FOUND)
                             ->writeHeader("Cache-Control", "max-age=10")
                             ->endProblemJson(error.dump());
                     } catch (...) {
                         res->writeStatus(HTTP_400_BAD_REQUEST)->end();
                         return;
                     }

                     nlohmann::json response = {{"ws_feed_id", feed_id}};

                     res->endJson(response.dump());

                     OBSERVER_TRACE(
                         "Opened camera connection in live view '{0}' as "
                         "'{1}' ",
                         uri, feed_id);
                 } else {
                     nlohmann::json error = {
                         {"title",
                          "Couldn't open a connection with the camera."}};

                     res->writeStatus(HTTP_400_BAD_REQUEST)
                         ->endProblemJson(error.dump());
                     OBSERVER_ERROR(
                         "Couldn't open live camera view, uri: '{0}'", uri);
                 }
             })
        /**
         * @brief does the same thing as requestCameraStream but providing
         * a path to the observer live view.
         */
        .get("/api/requestObserverStream",
             [&serverCtx, &observerCtx = serverCtx.recognizeContext](
                 auto* res, auto* req) {
                 auto uri = Web::LiveViewsManager<SSL>::observerUri;

                 if (observerCtx.running &&
                     serverCtx.liveViewsManager->CreateObserverView(uri)) {
                     std::string feed_id(
                         serverCtx.liveViewsManager->GetFeedId(uri));

                     nlohmann::json response = {{"ws_feed_id", feed_id}};

                     res->endJson(response.dump());
                 } else {
                     nlohmann::json error = {
                         {"title", "Observer is not running."}};

                     res->writeStatus(HTTP_400_BAD_REQUEST)
                         ->endProblemJson(error.dump());
                 }
             })

        .get("/api/configuration/",
             [&configurationDAO](auto* res, auto* req) {
                 // Get all the configurations id and name available
                 res->endJson(configurationDAO.GetAllNamesAndId().dump());
             })

        // create a new configuration
        .post(
            "/api/configuration/",
            [&configurationDAO](auto* res, auto* req) {
                res->onAborted([]() {});

                std::string buffer;

                // get it before we attach the reader
                auto ct = std::string(req->getHeader("content-type"));

                res->onData([&configurationDAO, req, res, ct,
                             buffer = std::move(buffer)](std::string_view data,
                                                         bool last) mutable {
                    buffer.append(data.data(), data.length());

                    if (last) {
                        //   std::cout << "buffer: " << buffer << std::endl;

                        if (ct != "application/json") {
                            res->writeStatus(HTTP_400_BAD_REQUEST)
                                ->end("Expected a json body");
                            return;
                        }

                        Observer::Configuration configuration;
                        nldb::json bufferAsJson;

                        // default response is an error
                        nldb::json response = {
                            {"status", 400},
                            {"title", "invalid configuration"},
                            {"detail",
                             "we couldn't parse the configuration, check "
                             "that it has all the fields needed"}};

                        try {
                            bufferAsJson = nlohmann::json::parse(buffer);

                            if (!bufferAsJson.empty()) {
                                // should clone
                                if (bufferAsJson.contains("clone_id")) {
                                    // move this code into a clone configuration
                                    // method that takes a json

                                    if (!bufferAsJson["clone_id"].is_string()) {
                                        res->writeStatus(HTTP_400_BAD_REQUEST)
                                            ->endProblemJson(
                                                (nlohmann::json {
                                                     {"title",
                                                      "clone_id must be of "
                                                      "type string"}})
                                                    .dump());
                                        return;
                                    }

                                    nldb::json config;
                                    try {
                                        config =
                                            configurationDAO.GetConfiguration(
                                                bufferAsJson["clone_id"]);
                                    } catch (const std::exception& e) {
                                        res->writeStatus(HTTP_404_NOT_FOUND)
                                            ->writeHeader("Cache-Control",
                                                          "max-age=5")
                                            ->endProblemJson(
                                                (nlohmann::json {
                                                     {"title",
                                                      "configuration not "
                                                      "found"}})
                                                    .dump());
                                        return;
                                    }

                                    // erase the id so it generates a new
                                    // one, also change its name to name +
                                    // "copy"

                                    config["name"] =
                                        config["name"].get<std::string>() +
                                        " copy";

                                    config.erase("_id");
                                    config.erase("id");

                                    bufferAsJson = config;
                                }

                                configuration = bufferAsJson;
                            } else {
                                bufferAsJson = configuration;
                            }

                            std::string id =
                                configurationDAO.InsertConfiguration(
                                    bufferAsJson);

                            // set success response
                            response = {{"id", id}};
                            res->writeHeader("Content-Type",
                                             "application/json");
                        } catch (const std::exception& e) {
                            OBSERVER_WARN("Configuration wasn't added: {}",
                                          e.what());

                            res->writeStatus(HTTP_400_BAD_REQUEST);
                            res->writeHeader("Content-Type",
                                             "application/problem+json");
                        }

                        res->end(response.dump());
                    }
                });
            })

        // clone a configuration
        // Note: temporarily moved to the function above
        /*.post(
            "/api/configurationClone",
            [&configurationDAO](auto* res, auto* req) {
                res->onAborted([]() {});

                std::string buffer;

                res->onData([&configurationDAO, res,
                             buffer = std::move(buffer)](std::string_view
                             data,
                                                         bool last) mutable {
                    buffer.append(data.data(), data.length());

                    if (last) {
                        nlohmann::json parsed;

                        try {
                            parsed = nlohmann::json::parse(buffer);
                        } catch (const std::exception& e) {
        OBSERVER_WARN("Couldn't parse body into JSON: {}",
                                        e.what());
                            res->writeStatus(HTTP_400_BAD_REQUEST)
                                ->endProblemJson(
                                    (nlohmann::json {
                                         {"title",
                                          "Couldn't parse body into JSON"}})
                                        .dump());
                            return;
                        }

                        if (!parsed.contains("id") ||
                            !parsed["id"].is_string()) {
                            res->writeStatus(HTTP_400_BAD_REQUEST)
                                ->endProblemJson(
                                    (nlohmann::json {{"title",
                                                      "body must contain "
                                                      "'id' of type
                                                      string"}})
                                        .dump());
                            return;
                        }

                        nldb::json config;
                        try {
                            config =
                                configurationDAO.GetConfiguration(parsed["id"]);
                        } catch (const std::exception& e) {
                            res->writeStatus(HTTP_404_NOT_FOUND)
                                ->endProblemJson(
                                    (nlohmann::json {
                                         {"title", "configuration not
                                         found"}})
                                        .dump());
                            return;
                        }

                        try {
                            // erase the id so it generates a new one, also
                            // change its name to name + "copy"

                            config["name"] =
                                config["name"].get<std::string>() + "copy";

                            config.erase("_id");
                            config.erase("id");

                            std::string newID =
                                configurationDAO.InsertConfiguration(config);

                            res->endJson(
                                (nlohmann::json {{"id", newID}}).dump());
                        } catch (const std::exception& e) {
                            res->writeStatus(HTTP_404_NOT_FOUND)
                                ->endProblemJson(
                                    (nlohmann::json {{"title",
                                                      "could not insert the "
                                                      "cloned
                                                      configuration"}})
                                        .dump());
                            return;
                        }
                    }
                });
            })*/

        // create a new camera
        .post(
            "/api/configuration/:id/camera/",
            [&configurationDAO](auto* res, auto* req) {
                res->onAborted([]() {});

                std::string buffer;

                auto ct = std::string(req->getHeader("content-type"));

                res->onData([&configurationDAO, res, req, ct,
                             buffer = std::move(buffer)](std::string_view data,
                                                         bool last) mutable {
                    buffer.append(data.data(), data.length());

                    if (last) {
                        if (ct != "application/json") {
                            res->writeStatus(HTTP_400_BAD_REQUEST)
                                ->end("Expected a json body");
                            return;
                        }

                        nlohmann::json parsed;

                        try {
                            parsed = nlohmann::json::parse(buffer);
                        } catch (const std::exception& e) {
                            res->writeStatus(HTTP_400_BAD_REQUEST)
                                ->endProblemJson(
                                    (nlohmann::json {
                                         {"title", "body isn't a valid json"}})
                                        .dump());
                            return;
                        }

                        auto configurationID =
                            std::string(req->getParameter(0));

                        // creat a instance with the default value
                        Observer::CameraConfiguration cam;
                        nldb::json jsonCamera;

                        if (parsed.contains("clone_id")) {
                            // clone the camera
                            if (!parsed["clone_id"].is_string()) {
                                res->writeStatus(HTTP_400_BAD_REQUEST)
                                    ->endProblemJson(
                                        (nlohmann::json {{"title",
                                                          "clone_id must be of "
                                                          "type string"}})
                                            .dump());
                                return;
                            }

                            try {
                                jsonCamera = configurationDAO.GetCamera(
                                    parsed["clone_id"]);

                                // erase id
                                jsonCamera.erase("_id");
                                jsonCamera.erase("id");

                                // change name to copy
                                jsonCamera["name"] =
                                    jsonCamera["name"].get<std::string>() +
                                    " copy";

                            } catch (const std::exception& e) {
                                res->writeStatus(HTTP_400_BAD_REQUEST)
                                    ->endProblemJson(
                                        (nlohmann::json {
                                             {"title",
                                              "camera to clone not found"}})
                                            .dump());
                                return;
                            }
                        } else {
                            cam.name = "new camera";

                            // parse the default camera to json
                            jsonCamera = cam;
                        }

                        try {
                            std::string newCameraID =
                                configurationDAO.AddCameraToConfiguration(
                                    configurationID, jsonCamera);

                            res->endJson(
                                (nlohmann::json {{"id", newCameraID}}).dump());
                        } catch (const std::exception& e) {
                            OBSERVER_WARN("Could not add the new camera: {}",
                                          e.what());

                            res->writeStatus(HTTP_400_BAD_REQUEST)
                                ->endProblemJson(
                                    (nlohmann::json {
                                         {"title",
                                          "couldn't add the new camera"}})
                                        .dump());
                        }
                    }
                });
            })

        // delete a configuration
        .del("/api/configuration/:id",
             [&configurationDAO](auto* res, auto* req) {
                 auto configID = std::string(req->getParameter(0));
                 configurationDAO.DeleteConfiguration(configID);

                 res->writeStatus(HTTP_204_NO_CONTENT)->end();
             })

        // delete a camera
        .del("/api/configuration/:id/camera/:cam_id",
             [&configurationDAO](auto* res, auto* req) {
                 auto configID = std::string(req->getParameter(0));
                 auto cameraID = std::string(req->getParameter(1));

                 try {
                     configurationDAO.DeleteCameraFromConfiguration(configID,
                                                                    cameraID);
                 } catch (const std::exception& e) {
                     res->writeStatus(HTTP_400_BAD_REQUEST)
                         ->endProblemJson(
                             (nlohmann::json {
                                  {"title", "could not delete this camera"}})
                                 .dump());

                     return;
                 }

                 res->writeStatus(HTTP_204_NO_CONTENT)->end();
             })

        .get("/api/configuration/:id",
             [&configurationDAO](auto* res, auto* req) {
                 std::string url(req->getUrl());
                 auto id = req->getParameter(0);
                 std::string fieldPath(req->getQuery("field"));
                 fieldPath = fieldPath;

                 nldb::json obj;
                 try {
                     obj = configurationDAO.GetConfiguration(std::string(id));
                 } catch (const std::exception& e) {
                     res->writeStatus(HTTP_404_NOT_FOUND)
                         ->writeHeader("Cache-Control", "max-age=5")
                         ->end("Configuration not found");
                     return;
                 }

                 // Get the camera from the database if requested
                 auto camerasPos = fieldPath.find("cameras/");
                 if (fieldPath.find("cameras/") != fieldPath.npos) {
                     fieldPath = fieldPath.substr(camerasPos + 8 /*cameras/*/);
                     auto nextSlash = fieldPath.find("/");
                     if (nextSlash != fieldPath.npos) {
                         // get the camera
                         auto cameraID = fieldPath.substr(0, nextSlash);
                         try {
                             obj = configurationDAO.GetCamera(cameraID);
                         } catch (const std::exception& e) {
                             res->writeStatus(HTTP_404_NOT_FOUND)
                                 ->writeHeader("Cache-Control", "max-age=5")
                                 ->end("Camera not found");
                             return;
                         }

                         // remove the id from the path
                         fieldPath = fieldPath.substr(nextSlash);
                     }
                 }

                 nldb::json value;
                 try {
                     value = Observer::ConfigurationParser::
                         GetConfigurationFieldValue(obj, fieldPath);
                 } catch (const std::exception& e) {
                     std::cout
                         << "Couldn't get configuration field: " << e.what()
                         << std::endl;
                     res->writeStatus(HTTP_400_BAD_REQUEST)->end();
                     return;
                 }

                 if (value.is_null()) {
                     res->writeStatus(HTTP_404_NOT_FOUND)
                         ->writeHeader("Cache-Control", "max-age=5")
                         ->end();
                     return;
                 }

                 res->endJson(value.dump());
             })

        .post(
            "/api/configuration/:id",
            [&configurationDAO](auto* res, auto* req) {
                res->onAborted([]() {});

                std::string buffer;

                // get it before we attach the reader
                auto ct = std::string(req->getHeader("content-type"));

                res->onData([&configurationDAO, res, req, ct,
                             buffer = std::move(buffer)](std::string_view data,
                                                         bool last) mutable {
                    buffer.append(data.data(), data.length());

                    if (last) {
                        if (ct != "application/json") {
                            res->writeStatus(HTTP_400_BAD_REQUEST)
                                ->end("Expected a json body");
                            return;
                        }

                        nlohmann::json parsed;
                        try {
                            parsed = nlohmann::json::parse(buffer);
                        } catch (const std::exception& e) {
                            res->writeStatus(HTTP_400_BAD_REQUEST)
                                ->end("Body is not a valid json");
                            return;
                        }

                        if (parsed.is_null() || !parsed.contains("field") ||
                            !parsed.contains("value") ||
                            !parsed["field"].is_string()) {
                            res->writeStatus(HTTP_400_BAD_REQUEST)
                                ->end(
                                    "Expected json members: field and "
                                    "value");
                            return;
                        }

                        std::string lastPathKey;
                        std::string fieldPath =
                            parsed["field"].get<std::string>();

                        auto camerasPos = fieldPath.find("cameras/");
                        if (fieldPath.find("cameras/") != fieldPath.npos) {
                            // UPDATE A CAMERA
                            fieldPath =
                                fieldPath.substr(camerasPos + 8 /*cameras/*/);
                            auto nextSlash = fieldPath.find("/");
                            if (nextSlash != fieldPath.npos) {
                                // get the camera
                                auto cameraID = fieldPath.substr(0, nextSlash);

                                // remove the id from the path
                                fieldPath = fieldPath.substr(nextSlash);

                                // TODO: ensure that cameraID belongs to
                                // parameter.id

                                nldb::json updated = Web::GenerateJsonFromPath(
                                    fieldPath, parsed["value"], &lastPathKey);

                                try {
                                    configurationDAO.UpdateCamera(cameraID,
                                                                  updated);
                                } catch (const nldb::WrongPropertyType& e) {
                                    nldb::json problem = {
                                        {"title", "Wrong property type"},
                                        {"invalidParams",
                                         {{lastPathKey,
                                           {{"reason", e.what()}}}}}};

                                    res->writeStatus(HTTP_400_BAD_REQUEST)
                                        ->writeHeader(
                                            "Content-Type",
                                            "application/problem+json")
                                        ->end(problem.dump());
                                } catch (...) {
                                    res->writeStatus(
                                           HTTP_500_INTERNAL_SERVER_ERROR)
                                        ->end();
                                }
                            }
                        } else {
                            // UPDATE A CONFIGURATION
                            std::string id(req->getParameter(0));

                            nldb::json updated = Web::GenerateJsonFromPath(
                                fieldPath, parsed["value"], &lastPathKey);

                            try {
                                configurationDAO.UpdateConfiguration(id,
                                                                     updated);
                            } catch (const nldb::WrongPropertyType& e) {
                                nldb::json problem = {
                                    {"title", "Wrong property type"},
                                    {"invalidParams",
                                     {{lastPathKey, {{"reason", e.what()}}}}}};

                                res->writeStatus(HTTP_400_BAD_REQUEST)
                                    ->writeHeader("Content-Type",
                                                  "application/problem+json")
                                    ->end(problem.dump());
                            } catch (...) {
                                res->writeStatus(HTTP_500_INTERNAL_SERVER_ERROR)
                                    ->end();
                            }
                        }

                        res->end("field updated");
                    }
                });
            })

        .get("/api/getCameraDefaults",
             [&configurationDAO](auto* res, auto* req) {
                 std::string uri(req->getQuery("uri"));

                 if (uri.empty()) {
                     // then try to get it from the database with:
                     std::string camera_id(req->getQuery("camera_id"));

                     if (camera_id.empty()) {
                         res->writeStatus(HTTP_400_BAD_REQUEST)->end();
                         return;
                     }

                     try {
                         auto camera = configurationDAO.GetCamera(camera_id);
                         uri = camera["url"];
                     } catch (const std::exception& e) {
                         nlohmann::json response = {
                             {"title", "Camera not found"}};

                         res->writeStatus(HTTP_404_NOT_FOUND)
                             ->writeHeader("Cache-Control", "max-age=5")
                             ->endProblemJson(response.dump());
                     }
                 }

                 Observer::VideoSource cap;
                 cap.Open(uri);

                 if (cap.isOpened()) {
                     double fps = cap.GetFPS();
                     Observer::Size size = cap.GetSize();

                     nlohmann::json response = {{"fps", fps}, {"size", size}};

                     res->endJson(response.dump());

                     cap.Close();
                 } else {
                     nlohmann::json response = {
                         {"title", "Camera not avilable"}};

                     res->writeStatus(HTTP_404_NOT_FOUND)
                         ->writeHeader("Cache-Control", "max-age=5")
                         ->endProblemJson(response.dump());
                 }
             })

        .get(
            "/api/getCameraFrame",
            [&configurationDAO, &cachedCameraImage](auto* res, auto* req) {
                std::string uri(req->getQuery("uri"));

                if (uri.empty()) {
                    // then try to get it from the database with:
                    std::string camera_id(req->getQuery("camera_id"));

                    if (camera_id.empty()) {
                        res->writeStatus(HTTP_400_BAD_REQUEST)->end();
                        return;
                    }

                    try {
                        auto camera = configurationDAO.GetCamera(camera_id);
                        uri = camera["url"];
                    } catch (const std::exception& e) {
                        nlohmann::json response = {
                            {"title", "Camera not found"}};

                        res->writeStatus(HTTP_404_NOT_FOUND)
                            ->writeHeader("Cache-Control", "max-age=5")
                            ->endProblemJson(response.dump());
                    }
                }

                if (!cachedCameraImage.contains(uri)) {
                    Observer::VideoSource cap;
                    Observer::Frame frame;

                    cap.Open(uri);
                    cap.GetNextFrame(frame);

                    int tries = 100;
                    while (frame.IsEmpty() && --tries) cap.GetNextFrame(frame);

                    if (tries <= 0) {
                        nlohmann::json response = {
                            {"title", "Camera didn't return any valid frames"}};

                        res->writeStatus(HTTP_500_INTERNAL_SERVER_ERROR)
                            ->endProblemJson(response.dump());
                        return;
                    }

                    std::vector<unsigned char> buffer;
                    frame.EncodeImage(".jpg", 90, buffer);

                    cachedCameraImage[uri] = std::move(buffer);
                }

                auto& buffer = cachedCameraImage[uri];
                res->writeHeader("content-type", "image/jpeg")
                    ->end(
                        std::string_view((char*)buffer.data(), buffer.size()));
            })

        .get("/api/camera/:id",
             [&configurationDAO](auto* res, auto* req) {
                 auto id = std::string(req->getParameter(0));

                 try {
                     nldb::json camera = configurationDAO.GetCamera(id);

                     //  TODO: We need to respond only with {name, id, url}
                     //  for that we should use camera repository
                     res->endJson(camera.dump());
                 } catch (const std::exception& e) {
                     nlohmann::json response = {{"title", "camera not found"}};
                     res->writeStatus(HTTP_404_NOT_FOUND)
                         ->writeHeader("Cache-Control", "max-age=5")
                         ->endProblemJson(response.dump());
                 }
             })

        .get("/api/start/:config_id",
             [&configurationDAO, &observerCtx = serverCtx.recognizeContext,
              &startRecognize](auto* res, auto* req) {
                 if (observerCtx.running) {
                     nlohmann::json response = {
                         {"title", "Observer is already running"}};
                     res->writeStatus(HTTP_404_NOT_FOUND)
                         ->writeHeader("Cache-Control", "max-age=5")
                         ->endProblemJson(response.dump());
                     return;
                 }

                 auto id = req->getParameter(0);
                 observerCtx.running_config_id = std::string(id);

                 nldb::json obj;
                 try {
                     obj = configurationDAO.GetConfiguration(std::string(id));
                 } catch (const std::exception& e) {
                     nlohmann::json response = {
                         {"title", "Configuration not found"}};
                     res->writeStatus(HTTP_404_NOT_FOUND)
                         ->writeHeader("Cache-Control", "max-age=5")
                         ->endProblemJson(response.dump());
                     return;
                 }

                 Observer::Configuration cfg;
                 try {
                     cfg = obj;
                 } catch (const std::exception& e) {
                     nlohmann::json response = {
                         {"title", "Not a valid configuration"}};
                     res->writeStatus(HTTP_404_NOT_FOUND)
                         ->writeHeader("Cache-Control", "max-age=5")
                         ->endProblemJson(response.dump());
                     return;
                 }

                 startRecognize(cfg);

                 res->end(nlohmann::json(
                              Web::ObserverStatusDTO {
                                  .running = true,
                                  .config_id = observerCtx.running_config_id})
                              .dump());
             })

        .get("/api/stop",
             [&observerCtx = serverCtx.recognizeContext, &stopRecognize](
                 auto* res, auto* req) {
                 if (observerCtx.running) {
                     stopRecognize();
                 }

                 res->end(nlohmann::json(
                              Web::ObserverStatusDTO {
                                  .running = false, .config_id = std::nullopt})
                              .dump());
             })

        .get("/api/observerStatus",
             [&observerCtx = serverCtx.recognizeContext](auto* res, auto* req) {
                 bool running = observerCtx.running;
                 std::optional<std::string> cfg_id;
                 if (running) cfg_id = observerCtx.running_config_id;

                 res->end(nlohmann::json(
                              Web::ObserverStatusDTO {.running = running,
                                                      .config_id = cfg_id})
                              .dump());
             })

        .get("/",
             [](auto* res, auto* req) {
                 std::cout << "Index!" << std::endl;

                 std::string rangeHeader(req->getHeader("range"));

                 FileStreamer::GetInstance().streamFile(res, "/web/index.html",
                                                        rangeHeader);
             })

        .get("/*.*",
             [](auto* res, auto* req) {
                 std::string url(req->getUrl());
                 std::string rangeHeader(req->getHeader("range"));

                 // search file only on the web folder
                 url = "/web/" + url;

                 if (!hasExtension(url)) {
                     req->setYield(true);  // mark as not handled
                 } else if (FileStreamer::GetInstance().streamFile(
                                res, url, rangeHeader)) {
                     // std::cout << "Successfully sended file" << std::endl;
                 } else {
                     res->end();
                 }
             })

        // setup preflight requests
        .options("/*",
                 [](auto* res, auto* req) {
                     res->writeStatus(HTTP_204_NO_CONTENT)

                         // allow all methods
                         ->writeHeader("Access-Control-Allow-Methods", "*")

                         ->writeHeader("Access-Control-Allow-Headers", "*")

                         // allow COR
                         ->writeHeader("Access-Control-Allow-Origin", "*")

                         // cache preflight at max, 24h
                         ->writeHeader("Access-Control-Max-Age", "86400")

                         ->tryEndWithoutContentLength({}, 0);
                 })

        .get("/*",
             [](auto* res, auto* req) {
                 res->writeStatus(HTTP_301_MOVED_PERMANENTLY)
                     ->writeHeader("Location", "/")
                     ->writeHeader("Content-Type", "text/html")
                     ->end();
             })
        // .ws<PerSocketData>(
        //     "/notifications",
        //     {.compression = uWS::CompressOptions::SHARED_COMPRESSOR,
        //      .open =
        //          [&serverCtx](auto* ws,
        //                       const std::list<std::string_view>& paths) {
        //              serverCtx.notificatorWS->AddClient(ws);
        //          },
        //      .close =
        //          [&serverCtx](auto* ws, int /*code*/,
        //                       std::string_view /*message*/) {
        //              serverCtx.notificatorWS->RemoveClient(ws);
        //          }})
        .ws<PerSocketData>(
            liveViewWsRoute,
            {.compression = uWS::DISABLED,
             .maxPayloadLength = 16 * 1024 * 1024,
             .idleTimeout = 16,
             .maxBackpressure = 1 * 1024 * 1024,
             .closeOnBackpressureLimit = false,
             .resetIdleTimeoutOnSend = false,
             .sendPingsAutomatically = true,
             .upgrade = nullptr,
             .open =
                 [&serverCtx](auto* ws,
                              const std::list<std::string_view>& paths) {
                     for (auto it = paths.begin(); it != paths.end(); it++)
                         std::cout << *it << ' ';
                     std::cout << '\n';
                     // 1° is live, 2° is *
                     std::string feedID(*std::next(paths.begin()));
                     ws->getUserData()->pathSubscribed = feedID;

                     OBSERVER_INFO("Client connected to live '{0}'", feedID);

                     if (serverCtx.liveViewsManager->Exists(feedID)) {
                         serverCtx.liveViewsManager->AddClient(ws);
                     } else {
                         OBSERVER_ERROR(
                             "Live feed wasn't initialized yet! path: {0}",
                             feedID);

                         ws->send(
                             "Wrong feed id, this one doesn't exists! "
                             "Closing "
                             "connection.");
                         ws->end();
                     }
                 },
             .close =
                 [&serverCtx](auto* ws, int /*code*/,
                              std::string_view /*message*/) {
                     OBSERVER_TRACE("Client disconnected from '{0}' !",
                                    ws->getUserData()->pathSubscribed);

                     serverCtx.liveViewsManager->RemoveClient(ws);
                 }})

        .ws<VideoBufferSocketData>(
            "/buffer/*",
            {
                .compression = uWS::DISABLED,
                .maxPayloadLength = 16 * 1024 * 1024,
                .idleTimeout = 16,
                .maxBackpressure = 1 * 1024 * 1024,
                .closeOnBackpressureLimit = false,
                .resetIdleTimeoutOnSend = false,
                .sendPingsAutomatically = true,
                .upgrade = nullptr,
                .open =
                    [&bufferWebSocket, &videoBufferRepository](
                        auto* ws, const std::list<std::string_view>& paths) {
                        // 1° is buffer, 2° is *
                        std::string bufferID(*std::next(paths.begin()));

                        if (videoBufferRepository.Exists(bufferID)) {
                            ws->getUserData()->bufferID = bufferID;

                            bufferWebSocket.AddClient(ws);

                            bufferWebSocket.SendInitialBuffer(
                                ws, videoBufferRepository.Get(bufferID).dump());
                        } else {
                            ws->end();
                        }
                    },
                .message =
                    [&videoBufferTasksManager](auto* ws,
                                               std::string_view message,
                                               uWS::OpCode opCode) {
                        if (uWS::OpCode::TEXT) {
                            if (message == "do_detection") {
                                videoBufferTasksManager.AddTask(
                                    Web::DoDetectionVideoBufferTask {
                                        .bufferID = ws->getUserData()->bufferID,
                                    });
                            }
                        }
                    },
                .close =
                    [&bufferWebSocket](auto* ws, int /*code*/,
                                       std::string_view /*message*/) {
                        bufferWebSocket.RemoveClient(ws);
                    },
            })

        .run();

    for (auto& funcThread : threads) {
        funcThread->Stop();
    }

    notificationsDB.close();
    configurationsDB.close();
}
