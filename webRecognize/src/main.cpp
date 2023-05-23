
#include <spdlog/fmt/bundled/format.h>

// nolitedb
#include "Constans.hpp"
#include "Controller/CameraController.hpp"
#include "Controller/LiveViewController.hpp"
#include "CronJobScheduler.hpp"
#include "DAL/File/ServerConfigurationPersistanceFile.hpp"
#include "DAL/INotificationRepository.hpp"
#include "DAL/NoLiteDB/NotificationRepositoryNLDB.hpp"
#include "DAL/NoLiteDB/VideoBufferRepositoryNLDB.hpp"
#include "DTO/DTONotificationDebugVideo.hpp"
#include "LiveVideo/LiveViewExceptions.hpp"
#include "Pattern/VideoBufferSubscriberPublisher.hpp"
#include "Server/ServerConfiguration.hpp"
#include "Utils/Filesystem.hpp"
#include "VideoBufferTasksManager.hpp"
#include "nldb/LOG/managers/log_constants.hpp"
#include "nldb/SQL3Implementation.hpp"

//
#include "Controller/CameraController.hpp"
#include "Controller/ConfigurationController.hpp"
#include "Controller/NotificationController.hpp"
#include "Controller/ObserverController.hpp"
#include "Controller/ServerConfigurationController.hpp"
#include "Controller/VideoBufferController.hpp"
#include "Controller/WebsocketVideoBufferController.hpp"
#include "DAL/ConfigurationDAO.hpp"
#include "LiveVideo/CameraLiveVideo.hpp"
#include "LiveVideo/LiveVideo.hpp"
#include "LiveVideo/ObserverLiveVideo.hpp"
#include "Serialization/JsonAvailableConfigurationDTO.hpp"
#include "Serialization/JsonSerialization.hpp"
#include "Server/ServerConfigurationProvider.hpp"
#include "Utils/StringUtils.hpp"
#include "Utils/VideoBuffer.hpp"
#include "nldb/backends/sqlite3/DB/DB.hpp"
#include "observer/Blob/BlobDetector/Blob.hpp"
#include "observer/Blob/BlobDetector/Finding.hpp"
#include "observer/Domain/Configuration/CameraConfiguration.hpp"
#include "observer/Domain/Configuration/ConfigurationParser.hpp"
#include "observer/Domain/EventValidator.hpp"
#include "observer/Domain/ObserverCentral.hpp"
#include "observer/Instrumentation/Instrumentation.hpp"
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
#include "CL/ConfigurationDAOCache.hpp"
#include "CL/NotificationCL.hpp"
#include "Utils/JsonUtils.hpp"

const bool SSL = false;
namespace fs = std::filesystem;
namespace rc = Observer;
typedef cv::Mat TFrame;

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

    OBSERVER_INIT_INSTRUMENTATION();

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

    Web::CL::ConfigurationDAOCache configurationDAOCache(
        std::make_unique<Web::DAL::ConfigurationDAO>(&configurationsDB));
    Web::Controller::ConfigurationController<SSL> configurationController(
        &app, &configurationDAOCache);

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
    Web::VideoBufferTasksManager videoBufferTasksManager(
        &videoBufferRepository, &configurationDAOCache);
    Web::Controller::VideoBufferController<SSL> videoBufferController(
        &app, &videoBufferTasksManager, &videoBufferRepository,
        &configurationDAOCache, &bufferWebSocket);

    /* ---------------- SERVER CONFIGURATION ---------------- */
    Web::DAL::ServerConfigurationPersistanceFile serverConfigPers(
        "server_configuration.json");
    Web::ServerConfigurationProvider::Initialize(&serverConfigPers);
    Web::Controller::ServerConfigurationController<SSL> serverCfgCntrl(&app);

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
        &notificationCache, &configurationDAOCache, &serverCtx);

    /* ----------------- CAMERAS REPOSITORY ----------------- */
    Web::DAL::CameraRepositoryMemory cameraRepository;

    Web::Controller::CameraController<SSL> cameraController(
        &app, &configurationDAOCache, &serverCtx);

    /* ----------------- START FUNCTIONALITY ---------------- */
    videoBufferTasksManager.SubscribeToTaskResult(&bufferWebSocket);

    videoBufferTasksManager.Start();
    bufferWebSocket.Start();

    threads.push_back(&videoBufferTasksManager);
    threads.push_back(&bufferWebSocket);

    /* ---------------------- LIVE VIEW --------------------- */
    Web::Controller::LiveViewController<SSL> liveViewController(&app,
                                                                &serverCtx);

    /* ------------------- CREATE OBSERVER ------------------ */
    // this might throw
    const auto startRecognize = [&observerCtx = serverCtx.recognizeContext,
                                 &notificationController,
                                 &notificationRepository,
                                 &threads](Observer::Configuration& cfg) {
        // modify it so all the configurations use the same folder
        cfg.mediaFolderPath =
            Web::ServerConfigurationProvider::Get().mediaFolder;

        if (observerCtx.observer)
            OBSERVER_WARN("Observer wasn't null at start");

        observerCtx.observer = std::make_unique<Observer::ObserverCentral>(
            cfg, notificationRepository.GetLastGroupID());

        observerCtx.observer->SubscribeToNewNotifications(
            (Observer::INotificationEventSubscriber*)&notificationController);

        // call it after it started or we will be subscribing to nothing
        observerCtx.observer->OnStartFinished(
            [&notificationController, &observerCtx]() {
                if (Web::ServerConfigurationProvider::Get()
                        .SaveNotificationDebugVideo) {
                    observerCtx.observer->SubscribeToValidCameraEvents(
                        (Observer::
                             IEventValidatorSubscriber*)&notificationController,
                        Observer::Priority::HIGH);
                }
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

    Web::Controller::ObserverController<SSL> observerController(
        &app, &serverCtx, &configurationDAOCache, std::move(startRecognize),
        std::move(stopRecognize));

    /* ---------------------- CRON JOBS --------------------- */
    Web::CronJobScheduler cronJobs;

    cronJobs.Add(
        "remove old notifications", 60 * 60 * 8 /*8hs*/, true,
        [&notificationRepository]() {
            const double days =
                Web::ServerConfigurationProvider::Get()
                    .notificationCleanupFilter.deleteIfOlderThanDays;

            const double d90 = days * 24 /*hours*/ * 60 /*min*/ * 60 /*sec*/;

            // older than 90 is date < now - 90 days
            const auto older = notificationRepository.GetBetweenDates(
                0, time(0) - d90, 100, true);

            for (auto& old : older) {
                std::tm* ptm = std::localtime(&old.datetime);
                char buffer[32];
                // Format: Mo, 15.06.2009 20:20:00
                std::strftime(buffer, 32, "%a, %d.%m.%Y %H:%M:%S", ptm);

                OBSERVER_TRACE("Removed old notification, date: {}", buffer);

                if (old.type == "image" || old.type == "video")
                    std::filesystem::remove(old.content);

                notificationRepository.Remove(old.notificationID);
            }
        });

    cronJobs.Add(
        "remove non reclaimed notification debug video", 60 * 2 /*2min*/, true,
        [&notificationRepository]() {
            const auto videoBufferFolder =
                std::filesystem::path(
                    Web::ServerConfigurationProvider::Get().mediaFolder) /
                Web::Constants::NOTIF_DEBUG_VIDEO_FOLDER;

            const long long sizeMB =
                Web::Filesystem::getDirectorySize(videoBufferFolder) / 1000 /
                1000;

            const long long maxSizeMB =
                Web::ServerConfigurationProvider::Get()
                    .notificationDebugVideoFilter.keepTotalNotReclaimedBelowMB;

            OBSERVER_TRACE("Directory size: {} MB of MAX {} MB", sizeMB,
                           maxSizeMB);

            if (sizeMB > maxSizeMB) {
                const auto nonReclaimed =
                    notificationRepository.GetNonReclaimedDebugVideos(100,
                                                                      true);
                size_t lastDeleted = 0;

                while (lastDeleted < nonReclaimed.size() &&
                       (Web::Filesystem::getDirectorySize(videoBufferFolder) /
                        1000 / 1000) > maxSizeMB) {
                    const auto current = nonReclaimed[lastDeleted++];

                    std::filesystem::remove(current.filePath);

                    OBSERVER_TRACE("Removed a debug video");

                    notificationRepository.RemoveDebugVideoEntry(current.id);
                }
            }
        });

    cronJobs.Start();

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
                     // std::cout << "Successfully sended file" <<
                     // std::endl;
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

        .run();

    for (auto& funcThread : threads) {
        funcThread->Stop();
    }

    notificationsDB.close();
    configurationsDB.close();

    OBSERVER_STOP_INSTRUMENTATION();
}
