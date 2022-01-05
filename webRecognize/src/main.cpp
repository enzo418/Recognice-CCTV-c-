
#include "../../recognize/Observer/Implementations/opencv/Implementation.hpp"
#include "../../recognize/Observer/src/Domain/ObserverCentral.hpp"
#include "../uWebSockets/src/App.h"
#include "../uWebSockets/src/HttpContextData.h"
#include "../uWebSockets/src/Multipart.h"
#include "server_utils.hpp"
#include "stream_content/FileExtension.hpp"
#include "stream_content/FileReader.hpp"
#include "stream_content/FileStreamer.hpp"

// Selects a Region of interes from a camera fram
#include <cstring>
#include <filesystem>

// #include "AreaSelector.hpp"
#include "Notifications/NotificationEndPoints.hpp"
#include "Server/ServerContext.hpp"
#include "base64.hpp"

namespace fs = std::filesystem;
namespace rc = Observer;
typedef cv::Mat TFrame;

int main(int argc, char** argv) {
    // initialize logger
    Observer::LogManager::Initialize();

    // recognizer instance
    std::shared_ptr<rc::ObserverCentral<TFrame>> recognizer;

    bool recognizerRunning = false;

    auto app = uWS::App();

    Web::NotificationsContext notificationsCtx = {
        .socketTopic = "notifications",
        .textEndpoint = "/addTextNotification",
        .imageEndpoint = "/addImageNotification",
        .videoEndpoint = "/addVideoNotification",
    };

    SetNotificationsEndPoints(app, notificationsCtx);

    Web::ServerContext serverCtx = {
        .rootFolder = fs::current_path() / "web",
        .port = 3001,
    };

    app.listen(
           serverCtx.port,
           [&serverCtx](auto* token) {
               if (token) {
                   OBSERVER_INFO(
                       "Serving folder {0} over HTTP at http://localhost:{1}",
                       serverCtx.rootFolder, serverCtx.port);
               }
           })
        .run();
}
