
#include <spdlog/fmt/bundled/format.h>

#include "../../recognize/Observer/Implementations/opencv/Implementation.hpp"
#include "../../recognize/Observer/src/Domain/Configuration/ConfigurationParser.hpp"
#include "../../recognize/Observer/src/Domain/ObserverCentral.hpp"
#include "../uWebSockets/src/App.h"
#include "../uWebSockets/src/HttpContextData.h"
#include "../uWebSockets/src/Multipart.h"
#include "server_types.hpp"
#include "server_utils.hpp"
#include "stream_content/FileExtension.hpp"
#include "stream_content/FileReader.hpp"
#include "stream_content/FileStreamer.hpp"
#include "stream_content/LiveVideo/CameraLiveVideo.hpp"
#include "stream_content/LiveVideo/LiveVideo.hpp"
#include "stream_content/LiveVideo/ObserverLiveVideo.hpp"

// Selects a Region of interes from a camera fram
#include <cstring>
#include <filesystem>
#include <memory>
#include <string_view>

// #include "AreaSelector.hpp"
#include "Notifications/NotificationEndPoints.hpp"
#include "Server/ServerContext.hpp"
#include "SocketData.hpp"
#include "base64.hpp"

const bool SSL = false;
namespace fs = std::filesystem;
namespace rc = Observer;
typedef cv::Mat TFrame;

constexpr std::string_view pathObserverLiveView = "feed_observer";
constexpr std::string_view prefixPathCameraLiveView = "feed_";

int main(int argc, char** argv) {
    // initialize logger
    Observer::LogManager::Initialize();

    if (argc <= 1) {
        OBSERVER_ERROR(
            "Missing configuration file argument.\n"
            "Usage: ./webRecognize ./config_test.yml");
        return 1;
    }

    // recognizer instance
    std::shared_ptr<rc::ObserverCentral<TFrame>> recognizer;

    std::vector<std::pair<IFunctionality*, std::thread>> threads;

    auto app = uWS::App();

    Web::NotificationsContext notificationsCtx = {
        .socketTopic = "notifications",
        .textEndpoint = "/addTextNotification",
        .imageEndpoint = "/addImageNotification",
        .videoEndpoint = "/addVideoNotification",
    };

    SetNotificationsEndPoints(app, notificationsCtx);

    Web::ServerContext<TFrame, SSL> serverCtx = {
        .rootFolder = fs::current_path() / "web",
        .port = 3001,
        .recognizeContext = {true, nullptr},
        .liveCamerasContext =
            std::make_unique<Web::LiveCamerasContext<TFrame, SSL>>()};

    FileStreamer fileStreamer(serverCtx.rootFolder);

    auto cfg = Observer::ConfigurationParser::ParseYAML(argv[1]);

    Observer::ObserverCentral<TFrame> observer(cfg);
    serverCtx.recognizeContext.observer = &observer;

    Observer::VideoSource<TFrame> cap;
    cap.Open(cfg.camerasConfiguration[0].url);
    double fps = cap.GetFPS();
    cap.Close();
    threads.push_back(
        {&observer,
         std::thread(&Observer::ObserverCentral<TFrame>::Start, &observer)});

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
             [&fileStreamer](auto* res, auto* req) {
                 std::cout << "Index!" << std::endl;

                 std::string rangeHeader(req->getHeader("range"));

                 fileStreamer.streamFile(res, "/index.html", rangeHeader);
             })
        .get(
            "/api/requestCameraStream",
            [&serverCtx](auto* res, auto* req) {
                res->writeHeader("Content-Type", "application/json");
                std::string uri(req->getQuery("uri"));

                std::string feed_id;

                if (serverCtx.liveCamerasContext->mapUriToFeed.contains(uri)) {
                    feed_id = serverCtx.liveCamerasContext->mapUriToFeed[uri];
                } else {
                    int id =
                        serverCtx.liveCamerasContext->camerasLiveView.size();

                    feed_id =
                        fmt::format("{0}{1}", prefixPathCameraLiveView, id);
                    auto cam =
                        std::make_unique<Web::CameraLiveVideo<TFrame, SSL>>(
                            id, uri, 90);

                    if (Observer::has_flag(cam->GetStatus(),
                                           Web::LiveViewStatus::ERROR)) {
                        res->end(GetErrorAlertReponse(
                            GetJsonString({{"error",
                                            "Couldn't open a connection with "
                                            "the camera."}})));
                        OBSERVER_ERROR(
                            "Couldn't open live camera view, uri: '{0}'", uri);
                    } else {
                        serverCtx.liveCamerasContext->camerasLiveView[feed_id] =
                            std::move(cam);

                        serverCtx.liveCamerasContext->mapUriToFeed[uri] =
                            feed_id;

                        OBSERVER_TRACE(
                            "Opened camera connection in live view '{0}' as "
                            "'{1}'",
                            uri, feed_id);
                    }
                }

                res->end(GetSuccessAlertReponse(
                    GetJsonString({{"ws_feed_path", feed_id}})));
            })
        .get("/api/requestObserverStream",
             [&serverCtx, &observer](auto* res, auto* req) {
                 res->writeHeader("Content-Type", "application/json");

                 if (!serverCtx.recognizeContext.running) {
                     res->end(GetErrorAlertReponse(GetJsonString(
                         {{"error", "recognize is not running"}})));
                 } else {
                     if (!serverCtx.observerLiveView) {
                         serverCtx.observerLiveView = std::make_unique<
                             Web::ObserverLiveVideo<TFrame, SSL>>(0, 5, 90);
                     }

                     observer.SubscribeToFrames(
                         serverCtx.observerLiveView.get());

                     res->end(GetSuccessAlertReponse(GetJsonString(
                         {{"ws_feed_path", pathObserverLiveView}})));
                 }
             })
        .ws<PerSocketData>(
            "/*",
            {/* Settings */
             .compression = uWS::SHARED_COMPRESSOR,
             .maxPayloadLength = 16 * 1024 * 1024,
             .idleTimeout = 16,
             .maxBackpressure = 1 * 1024 * 1024,
             .closeOnBackpressureLimit = false,
             .resetIdleTimeoutOnSend = false,
             .sendPingsAutomatically = true,
             /* Handlers */
             .upgrade = nullptr,
             .open =
                 [&serverCtx](auto* ws,
                              const std::list<std::string_view>& paths) {
                     for (auto it = paths.begin(); it != paths.end(); it++)
                         std::cout << *it << ' ';
                     std::cout << '\n';

                     std::string path(paths.front());
                     ws->getUserData()->pathSubscribed = std::string(path);

                     OBSERVER_INFO("Client connected to {0}", path);

                     if (path == pathObserverLiveView) {
                         // user requested observer live view
                         if (!serverCtx.observerLiveView) {
                             OBSERVER_ERROR(
                                 "Observer live view wasn't initialized yet! "
                                 "path: {0}",
                                 path);
                             return;
                         }

                         if (Observer::has_flag(
                                 serverCtx.observerLiveView->GetStatus(),
                                 Web::LiveViewStatus::STOPPED)) {
                             serverCtx.observerLiveView->Start();
                         }

                         serverCtx.observerLiveView->AddClient(ws);
                     } else if (path.starts_with(prefixPathCameraLiveView)) {
                         // user requested camera live view
                         if (!serverCtx.liveCamerasContext->camerasLiveView
                                  .contains(path)) {
                             OBSERVER_ERROR(
                                 "Camera wasn't initialized yet! path: {0}",
                                 path);
                             return;
                         }

                         auto camera =
                             serverCtx.liveCamerasContext->camerasLiveView[path]
                                 .get();

                         if (Observer::has_flag(camera->GetStatus(),
                                                Web::LiveViewStatus::STOPPED)) {
                             camera->Start();
                         }

                         camera->AddClient(ws);
                     }
                 },
             .close =
                 [&serverCtx, &notificationsCtx](auto* ws, int /*code*/,
                                                 std::string_view /*message*/) {
                     std::string path(ws->getUserData()->pathSubscribed);
                     ws->subscribe(notificationsCtx.socketTopic);

                     OBSERVER_TRACE("Client disconnected from '{0}'!", path);

                     Web::LiveVideo<TFrame, SSL>* liveView;

                     if (path == pathObserverLiveView) {
                         if (!serverCtx.observerLiveView) {
                             OBSERVER_ERROR(
                                 "Observer live veiw is not "
                                 "initialized on disconnect!!");
                         }

                         serverCtx.observerLiveView->RemoveClient(ws);

                         liveView = serverCtx.observerLiveView.get();
                     } else if (path.starts_with(prefixPathCameraLiveView)) {
                         if (!serverCtx.liveCamerasContext->camerasLiveView
                                  .contains(path)) {
                             OBSERVER_ERROR(
                                 "Camera is no initialized on disconnect!");
                         }

                         liveView =
                             serverCtx.liveCamerasContext->camerasLiveView[path]
                                 .get();

                         liveView->RemoveClient(ws);
                     }

                     if (liveView->GetTotalClients() == 0) {
                         liveView->Stop();
                         OBSERVER_TRACE(
                             "Stopping live view since there are 0 clients");
                     }
                 }})

        .run();

    for (auto& funcThread : threads) {
        std::get<0>(funcThread)->Stop();
        auto& thread = std::get<1>(funcThread);
        if (thread.joinable()) {
            thread.join();
        }
    }
}
