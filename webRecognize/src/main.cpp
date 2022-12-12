
#include <spdlog/fmt/bundled/format.h>

// nolitedb
#include "DAL/NoLiteDB/NotificationRepositoryNLDB.hpp"
#include "LiveVideo/LiveViewExceptions.hpp"
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
#include "nldb/backends/sqlite3/DB/DB.hpp"
#include "observer/Domain/Configuration/ConfigurationParser.hpp"
#include "observer/Domain/ObserverCentral.hpp"
#include "observer/Log/log.hpp"
#include "server_types.hpp"
#include "server_utils.hpp"
#include "stream_content/FileExtension.hpp"
#include "stream_content/FileReader.hpp"
#include "stream_content/FileStreamer.hpp"
#include "uWebSockets/App.h"
#include "uWebSockets/HttpContextData.h"
#include "uWebSockets/Multipart.h"

// Selects a Region of interes from a camera fram
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

int main(int argc, char** argv) {
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
        &app, &notificationRepository, &notificationCache);

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

    /* ------------------- CREATE OBSERVER ------------------ */
    const auto startRecognize = [&observerCtx = serverCtx.recognizeContext,
                                 &notificationController,
                                 &threads](const Observer::Configuration& cfg) {
        if (observerCtx.observer)
            OBSERVER_WARN("Observer wasn't null at start");

        observerCtx.observer = std::make_unique<Observer::ObserverCentral>(cfg);

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

    if (argc > 1) {
        auto cfg =
            Observer::ConfigurationParser::ConfigurationFromJsonFile(argv[1]);
        startRecognize(cfg);
    }

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
        .get("/",
             [](auto* res, auto* req) {
                 std::cout << "Index!" << std::endl;

                 std::string rangeHeader(req->getHeader("range"));

                 FileStreamer::GetInstance().streamFile(res, "/web/index.html",
                                                        rangeHeader);
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
        .post("/api/configuration/",
              [&configurationDAO](auto* res, auto* req) {
                  res->onAborted([]() {});

                  std::string buffer;

                  res->onData([&configurationDAO, res,
                               buffer = std::move(buffer)](
                                  std::string_view data, bool last) mutable {
                      buffer.append(data.data(), data.length());

                      if (last) {
                          //   std::cout << "buffer: " << buffer << std::endl;

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
                              if (!buffer.empty()) {
                                  // Replace escaped quotes with quotes
                                  Web::StringUtils::replaceSubstring(
                                      buffer, "\\\"", "\"");

                                  // "{name: ..., ...}" to {name: ..., ...}
                                  if (buffer.starts_with('"')) {
                                      bufferAsJson = nlohmann::json::parse(
                                          buffer.begin() + 1, buffer.end() - 1);
                                  } else {
                                      bufferAsJson =
                                          nlohmann::json::parse(buffer);
                                  }

                                  configuration = bufferAsJson;
                              } else {
                                  bufferAsJson = configuration;
                              }

                              auto ids = configurationDAO.InsertConfiguration(
                                  bufferAsJson);

                              // set success response
                              response = {{"id", ids[0]}};
                              res->writeHeader("Content-Type",
                                               "application/json");
                          } catch (const std::exception& e) {
                              std::cout
                                  << "Configuration wasn't added: " << e.what()
                                  << std::endl;

                              res->writeStatus(HTTP_400_BAD_REQUEST);
                              res->writeHeader("Content-Type",
                                               "application/problem+json");
                          }

                          res->end(response.dump());
                      }
                  });
              })
        .get("/api/configuration/:id",
             [&configurationDAO](auto* res, auto* req) {
                 std::string url(req->getUrl());
                 auto id = req->getParameter(0);
                 std::string fieldPath(req->getQuery("field"));
                 fieldPath = fieldPath;

                 nldb::json obj;
                 try {
                     obj = configurationDAO.Get(std::string(id));
                 } catch (const std::exception& e) {
                     res->writeStatus(HTTP_404_NOT_FOUND)
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
                     res->writeStatus(HTTP_404_NOT_FOUND)->end();
                     return;
                 }

                 res->endJson(value.dump());
             })

        .post(
            "/api/configuration/:id",
            [&configurationDAO](auto* res, auto* req) {
                res->onAborted([]() {});

                std::string buffer;

                res->onData([&configurationDAO, res, req,
                             buffer = std::move(buffer)](std::string_view data,
                                                         bool last) mutable {
                    buffer.append(data.data(), data.length());

                    if (last) {
                        // uWebSocket can't take POST with header???? wtf

                        /*std::string
                        contentType(req->getHeader("content-type")); if
                        (contentType != "application/json") {
                            res->writeStatus(HTTP_400_BAD_REQUEST)
                                ->end("Expected a json body");
                            return;
                        }*/

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

        .get("/api/start/:config_id",
             [&configurationDAO, &observerCtx = serverCtx.recognizeContext,
              &startRecognize](auto* res, auto* req) {
                 if (observerCtx.running) {
                     nlohmann::json response = {
                         {"title", "Observer is already running"}};
                     res->writeStatus(HTTP_404_NOT_FOUND)
                         ->endProblemJson(response.dump());
                     return;
                 }

                 auto id = req->getParameter(0);
                 observerCtx.running_config_id = std::string(id);

                 nldb::json obj;
                 try {
                     obj = configurationDAO.Get(std::string(id));
                 } catch (const std::exception& e) {
                     nlohmann::json response = {
                         {"title", "Configuration not found"}};
                     res->writeStatus(HTTP_404_NOT_FOUND)
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
                         ->endProblemJson(response.dump());
                     return;
                 }

                 startRecognize(cfg);

                 res->end(nlohmann::json(
                              ObserverStatusDTO {
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
                              ObserverStatusDTO {.running = false,
                                                 .config_id = std::nullopt})
                              .dump());
             })

        .get(
            "/api/observerStatus",
            [&observerCtx = serverCtx.recognizeContext](auto* res, auto* req) {
                bool running = observerCtx.running;
                std::optional<std::string> cfg_id;
                if (running) cfg_id = observerCtx.running_config_id;

                res->end(nlohmann::json(ObserverStatusDTO {.running = running,
                                                           .config_id = cfg_id})
                             .dump());
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
                     // std::cout << "Succesfull sended file" << std::endl;
                 } else {
                     res->end();
                 }
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

        .run();

    for (auto& funcThread : threads) {
        funcThread->Stop();
    }

    notificationsDB.close();
    configurationsDB.close();
}
