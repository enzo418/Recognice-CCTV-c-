
#include "../../recognize/Observer/Implementations/opencv/Implementation.hpp"
#include "../../recognize/Observer/src/Domain/Configuration/ConfigurationParser.hpp"
#include "../../recognize/Observer/src/Domain/ObserverCentral.hpp"
#include "../uWebSockets/src/App.h"
#include "../uWebSockets/src/HttpContextData.h"
#include "../uWebSockets/src/Multipart.h"
#include "server_utils.hpp"
#include "stream_content/FileExtension.hpp"
#include "stream_content/FileReader.hpp"
#include "stream_content/FileStreamer.hpp"
#include "stream_content/LiveVideo.hpp"

// Selects a Region of interes from a camera fram
#include <cstring>
#include <filesystem>
#include <string_view>

// #include "AreaSelector.hpp"
#include "Notifications/NotificationEndPoints.hpp"
#include "Server/ServerContext.hpp"
#include "base64.hpp"

const bool SSL = false;
namespace fs = std::filesystem;
namespace rc = Observer;
typedef cv::Mat TFrame;

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

    bool recognizerRunning = false;

    auto app = uWS::App();

    Web::NotificationsContext notificationsCtx = {
        .socketTopic = "notifications",
        .textEndpoint = "/addTextNotification",
        .imageEndpoint = "/addImageNotification",
        .videoEndpoint = "/addVideoNotification",
    };

    SetNotificationsEndPoints(app, notificationsCtx);

    Web::ServerContext<TFrame> serverCtx = {
        .rootFolder = fs::current_path() / "web",
        .port = 3001,
        .recognizeContext = {false, nullptr}};

    FileStreamer fileStreamer(serverCtx.rootFolder);

    auto cfg = Observer::ConfigurationParser::ParseYAML(argv[1]);

    Observer::ObserverCentral<TFrame> observer(cfg);
    serverCtx.recognizeContext.observer = &observer;

    Observer::VideoSource<TFrame> cap;
    cap.Open(cfg.camerasConfiguration[0].url);
    double fps = cap.GetFPS();
    cap.Close();

    Web::LiveVideo<TFrame, SSL> liveVideo(0, fps, 90);

    threads.push_back(
        {&liveVideo,
         std::thread(&Web::LiveVideo<TFrame, SSL>::Start, &liveVideo)});

    threads.push_back(
        {&observer,
         std::thread(&Observer::ObserverCentral<TFrame>::Start, &observer)});

    observer.SubscribeToFrames(&liveVideo);

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
        .get("/api/requestVideoStream",
             [&liveVideo, &serverCtx](auto* res, auto* req) {
                 // 1. get type: single camera | observer
                 const auto type = req->getQuery("type");
                 if (type == "single_camera") {
                     // feedExists = servercontext.lives.feedExists(url)
                     // if feedExists
                     //     feedId = servercontext.lives.get(url)
                     // else:
                     //     feedId = servercontext.lives.create(url)
                     //
                     // res->send({feed_id: feedId})
                 } else if (type == "observer") {
                     if (!serverCtx.recognizeContext.running) {
                         // res->send(Error("Recognize is not running"))
                     } else {
                         // exists = servercontext.lives.feedExists("observer")
                         // if exists
                         //     servercontext.lives.get("observer")
                         // else:
                         //     servercontext.lives.create("observer")
                         // res->send({feed_id: "observer_feed"})
                     }
                 }
             })
        .ws<PerSocketData>(
            "/*", {/* Settings */
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
                       [&liveVideo](auto* ws, std::string_view path) {
                           /* Open event here, you may access
                            * ws->getUserData() which points to a
                            * PerSocketData struct */

                           // add to connected clients
                           liveVideo.AddClient(ws);

                           OBSERVER_INFO("Client connected to {0}", path);
                       },
                   .close =
                       [&liveVideo](auto* ws, int /*code*/,
                                    std::string_view /*message*/) {
                           /* You may access ws->getUserData() here */
                           liveVideo.RemoveClient(ws);
                           std::cout << "Client disconnected!\n";
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
