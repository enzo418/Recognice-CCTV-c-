
#include <spdlog/fmt/bundled/format.h>

#include "Controller/NotificationController.hpp"
#include "LiveVideo/CameraLiveVideo.hpp"
#include "LiveVideo/LiveVideo.hpp"
#include "LiveVideo/ObserverLiveVideo.hpp"
#include "Parsing/JsonNotification.hpp"
#include "observer/Domain/Configuration/ConfigurationParser.hpp"
#include "observer/Domain/ObserverCentral.hpp"
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

// #include "AreaSelector.hpp"
#include "Server/ServerContext.hpp"
#include "SocketData.hpp"
#include "base64.hpp"

// DAL
#include "DAL/InMemory/CameraRepository.hpp"
#include "DAL/InMemory/NotificationRepository.hpp"

// DTO
#include "DTO/json_dto_declarations.hpp"

// CL
#include "CL/NotificationCL.hpp"
#include "nldb/SQL3Implementation.hpp"

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
    nldb::LogManager::SetLevel(nldb::log_level::err);

    if (argc <= 1) {
        OBSERVER_ERROR(
            "Missing configuration file argument.\n"
            "Usage: ./webRecognize ./config_test.yml");
        return 1;
    }

    // recognizer instance
    std::shared_ptr<rc::ObserverCentral> recognizer;

    std::vector<IFunctionality*> threads;

    auto app = uWS::App();

    Web::ServerContext<SSL> serverCtx = {
        .rootFolder = fs::current_path(),
        .port = 3001,
        .recognizeContext = {true, nullptr},

        // this should not be like this, allocating on heap. This should be on a
        // class that manages all of this, like App or Server. But for now it
        // will stay like this until more features are added and become stable.

        .liveViewsManager = std::make_unique<Web::LiveViewsManager<SSL>>(
            OBSERVER_LIVE_VIEW_MAX_FPS, &serverCtx.recognizeContext)};

    FileStreamer::Init<SSL>(serverCtx.rootFolder);

    auto cfg = Observer::ConfigurationParser::ParseYAML(argv[1]);

    Observer::ObserverCentral observer(cfg);
    serverCtx.recognizeContext.observer = &observer;

    // notifications
    Web::DAL::NotificationRepositoryMemory notificationRepository;
    Web::CL::NotificationCL notificationCache(&notificationRepository);
    Web::Controller::NotificationController<SSL> notificationController(
        &app, &notificationRepository, &notificationCache);

    observer.SubscribeToNewNotifications(
        (Observer::INotificationEventSubscriber*)&notificationController);

    // cameras
    Web::DAL::CameraRepositoryMemory cameraRepository;

    for (auto&& cam : cfg.camerasConfiguration) {
        auto camera = Web::Domain::Camera(cam.name, cam.url);
        cameraRepository.Add(camera);
    }

    Observer::VideoSource cap;
    cap.Open(cfg.camerasConfiguration[0].url);
    double fps = cap.GetFPS();
    cap.Close();

    threads.push_back(&observer);

    observer.Start();

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
        .get("/test",
             [&notificationController](auto* res, auto* req) {
                 static int id = 0;
                 Observer::DTONotification ev(
                     id++, "prender_sin_boton2.mp4",
                     Observer::ENotificationType::VIDEO);
                 notificationController.update(ev);
                 res->end();
             })
        /**
         * @brief Request a live view of the camera. If successfully
         * generated the live view it returns a json with the ws_feed_path, to
         * wich the client can request a WebSocket connection and we will
         * provide the images through it
         */
        .get(
            "/api/requestCameraStream",
            [&serverCtx](auto* res, auto* req) {
                res->writeHeader("Content-Type", "application/json");
                std::string uri(req->getQuery("uri"));

                if (serverCtx.liveViewsManager->CreateCameraView(uri)) {
                    std::string feed_id(
                        serverCtx.liveViewsManager->GetFeedId(uri));

                    res->endJson(GetSuccessAlertReponse(GetJsonString(
                        {{"ws_feed_path", liveViewPrefix + feed_id, true}})));

                    OBSERVER_TRACE(
                        "Opened camera connection in live view '{0}' as '{1}' ",
                        uri, feed_id);
                } else {
                    res->endJson(GetErrorAlertReponse(
                        GetJsonString({{"error",
                                        "Couldn't open a connection with "
                                        "the camera.",
                                        true}})));
                    OBSERVER_ERROR("Couldn't open live camera view, uri: '{0}'",
                                   uri);
                }
            })
        /**
         * @brief does the same thing as requestCameraStream but provinding a
         * path to the observer live view.
         */
        .get("/api/requestObserverStream",
             [&serverCtx, &observer](auto* res, auto* req) {
                 res->writeHeader("Content-Type", "application/json");

                 auto uri = Web::LiveViewsManager<SSL>::observerUri;

                 if (!observer.IsRunning()) {
                     observer.Start();
                 }

                 if (serverCtx.liveViewsManager->CreateObserverView(uri)) {
                     std::string feed_id(
                         serverCtx.liveViewsManager->GetFeedId(uri));

                     res->endJson(GetSuccessAlertReponse(GetJsonString(
                         {{"ws_feed_path", liveViewPrefix + feed_id, true}})));
                 } else {
                     res->endJson(GetErrorAlertReponse(GetJsonString(
                         {{"error", "Observer might not be running.", true}})));
                 }
             })
        .get("/stream/notification/test11",
             [](auto* res, auto* req) {
                 std::string filename("prender_sin_boton2.mp4");
                 //  std::string id(req->getParameter(0));
                 std::string id = "test11";
                 std::string rangeHeader(req->getHeader("range"));

                 // f = notificationService.getFilename(id)
                 //     if cache.isCached(id):
                 //         return cache.at(id)
                 //     else
                 //         db.notification.select.where id = id
                 //
                 // fileStreamer.streamFile(res, f, rangeHeader)

                 std::cout << "id: " << id << std::endl;

                 FileStreamer::GetInstance().streamFile(res, filename,
                                                        rangeHeader);
             })
        .get("/api/configurationFiles",
             [](auto* res, auto* req) {
                 std::string url(req->getUrl());
                 std::string rangeHeader(req->getHeader("range"));

                 const auto paths = GetAvailableConfigurations(
                     {"../../recognize/build/", "./", "./configurations/"});

                 res->endJson(json_dto::to_json(paths));
             })

        .get("/api/configuration/:id",
             [&argv](auto* res, auto* req) {
                 std::string url(req->getUrl());
                 auto id = req->getParameter(0);
                 std::string fieldPath(req->getQuery("field"));
                 fieldPath = "configuration/" + fieldPath;

                 YAML::Node obj;
                 Observer::ConfigurationParser::ReadConfigurationObjectFromFile(
                     argv[1], obj);

                 std::cout << "field: " << fieldPath << std::endl;

                 YAML::Node rCfg;
                 Observer::ConfigurationParser::TryGetConfigurationFieldValue(
                     obj, fieldPath, rCfg);

                 if (rCfg.IsNull()) {
                     res->writeStatus(HTTP_404_NOT_FOUND)->end();
                 }

                 res->endJson(Observer::ConfigurationParser::NodeAsJson(rCfg));
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
                             "Wrong feed id, this one doesn't exists! Closing "
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
}
