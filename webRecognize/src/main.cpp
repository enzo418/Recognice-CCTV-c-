
#include <spdlog/fmt/bundled/format.h>

// nolitedb
#include "Controller/CameraController.hpp"
#include "DAL/INotificationRepository.hpp"
#include "DAL/NoLiteDB/NotificationRepositoryNLDB.hpp"
#include "DAL/NoLiteDB/VideoBufferRepositoryNLDB.hpp"
#include "DTO/DTONotificationDebugVideo.hpp"
#include "LiveVideo/LiveViewExceptions.hpp"
#include "Pattern/VideoBufferSubscriberPublisher.hpp"
#include "VideoBufferTasksManager.hpp"
#include "nldb/LOG/managers/log_constants.hpp"
#include "nldb/SQL3Implementation.hpp"

//
#include "Controller/CameraController.hpp"
#include "Controller/ConfigurationController.hpp"
#include "Controller/NotificationController.hpp"
#include "Controller/VideoBufferController.hpp"
#include "Controller/WebsocketVideoBufferController.hpp"
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
#include "observer/Domain/EventValidator.hpp"
#include "observer/Domain/ObserverCentral.hpp"
#include "observer/Log/log.hpp"
#include "observer/Pattern/ObserverBasics.hpp"
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

    /* ---------------- MAIN UWEBSOCKETS APP ---------------- */
    auto app = uWS::App();

    /* ----------------- CONFIGURATIONS DAO ----------------- */
    nldb::DBSL3 configurationsDB;
    if (!configurationsDB.open("configurations.db")) {
        OBSERVER_ERROR("Could not open the database!");
        configurationsDB.throwLastError();
    }

    Web::DAL::ConfigurationDAO configurationDAO(&configurationsDB);
    Web::Controller::ConfigurationController<SSL> configurationController(
        &app, &configurationDAO);

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
    Web::Controller::VideoBufferController<SSL> videoBufferController(
        &app, &videoBufferTasksManager, &videoBufferRepository,
        &configurationDAO, &bufferWebSocket);

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
        &app, &notificationRepository, &videoBufferRepository,
        &notificationCache, &configurationDAO, &serverCtx);

    /* ----------------- CAMERAS REPOSITORY ----------------- */
    Web::DAL::CameraRepositoryMemory cameraRepository;

    Web::Controller::CameraController<SSL> cameraController(&app,
                                                            &configurationDAO);

    /* ----------------- START FUNCTIONALITY ---------------- */
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

        // call it after it started or we will be subscribing to nothing
        observerCtx.observer->OnStartFinished([&notificationController,
                                               &observerCtx]() {
            observerCtx.observer->SubscribeToValidCameraEvents(
                (Observer::IEventValidatorSubscriber*)&notificationController,
                Observer::Priority::HIGH);
        });

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

        /**
         * @brief Request a live view of the camera. If successfully
         * generated the live view it returns a json with the ws_feed_path,
         * to which the client can request a WebSocket connection and we will
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

        .run();

    for (auto& funcThread : threads) {
        funcThread->Stop();
    }

    notificationsDB.close();
    configurationsDB.close();
}
