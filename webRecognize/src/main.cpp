
#include <jsoncpp/json/writer.h>
#include <spdlog/fmt/bundled/format.h>

#include "../../recognize/Observer/src/Domain/Configuration/ConfigurationParser.hpp"
#include "../../recognize/Observer/src/Domain/ObserverCentral.hpp"
#include "../uWebSockets/src/App.h"
#include "../uWebSockets/src/HttpContextData.h"
#include "../uWebSockets/src/Multipart.h"
#include "LiveVideo/CameraLiveVideo.hpp"
#include "LiveVideo/LiveVideo.hpp"
#include "LiveVideo/ObserverLiveVideo.hpp"
#include "server_types.hpp"
#include "server_utils.hpp"
#include "stream_content/FileExtension.hpp"
#include "stream_content/FileReader.hpp"
#include "stream_content/FileStreamer.hpp"

// Selects a Region of interes from a camera fram
#include <cstring>
#include <filesystem>
#include <memory>
#include <string_view>

// #include "AreaSelector.hpp"
#include "Server/ServerContext.hpp"
#include "SocketData.hpp"
#include "base64.hpp"

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
        .rootFolder = fs::current_path() / "web",
        .port = 3001,
        .recognizeContext = {true, nullptr},

        // this should not be like this, allocating on heap. This should be on a
        // class that manages all of this, like App or Server. But for now it
        // will stay like this until more features are added and become stable.

        .liveViewsManager = std::make_unique<Web::LiveViewsManager<SSL>>(
            OBSERVER_LIVE_VIEW_MAX_FPS, &serverCtx.recognizeContext),
        .notificatorWS = std::make_unique<Web::WebsocketNotificator<SSL>>()};

    FileStreamer fileStreamer(serverCtx.rootFolder);

    auto cfg = Observer::ConfigurationParser::ParseYAML(argv[1]);

    Observer::ObserverCentral observer(cfg);
    serverCtx.recognizeContext.observer = &observer;

    observer.SubscribeToNewNotifications(
        (Observer::INotificationEventSubscriber*)serverCtx.notificatorWS.get());

    Observer::VideoSource cap;
    cap.Open(cfg.camerasConfiguration[0].url);
    double fps = cap.GetFPS();
    cap.Close();

    threads.push_back(&observer);

    serverCtx.notificatorWS->Start();

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

                    res->end(GetSuccessAlertReponse(GetJsonString(
                        {{"ws_feed_path", liveViewPrefix + feed_id, true}})));

                    OBSERVER_TRACE(
                        "Opened camera connection in live view '{0}' as '{1}' ",
                        uri, feed_id);
                } else {
                    res->end(GetErrorAlertReponse(
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

                     res->end(GetSuccessAlertReponse(GetJsonString(
                         {{"ws_feed_path", liveViewPrefix + feed_id, true}})));
                 }
             })
        .get("/*.*",
             [&fileStreamer](auto* res, auto* req) {
                 std::string url(req->getUrl());
                 std::string rangeHeader(req->getHeader("range"));

                 if (!hasExtension(url)) {
                     req->setYield(true);  // mark as not handled
                 } else if (fileStreamer.streamFile(res, url, rangeHeader)) {
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
        .ws<PerSocketData>(
            "/notifications",
            {.compression = uWS::CompressOptions::SHARED_COMPRESSOR,
             .open =
                 [&serverCtx](auto* ws,
                              const std::list<std::string_view>& paths) {
                     serverCtx.notificatorWS->AddClient(ws);
                 },
             .close =
                 [&serverCtx](auto* ws, int /*code*/,
                              std::string_view /*message*/) {
                     serverCtx.notificatorWS->RemoveClient(ws);
                 }})
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
