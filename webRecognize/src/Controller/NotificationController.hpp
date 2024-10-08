#pragma once

#include <filesystem>
#include <future>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "../CL/NotificationCL.hpp"
#include "../DAL/INotificationRepository.hpp"
#include "../Domain/Camera.hpp"
#include "../stream_content/FileStreamer.hpp"
#include "Constans.hpp"
#include "DAL/IConfigurationDAO.hpp"
#include "DAL/NoLiteDB/VideoBufferRepositoryNLDB.hpp"
#include "DTO/DTONotification.hpp"
#include "Domain/Notification.hpp"
#include "Server/ServerConfigurationProvider.hpp"
#include "Server/ServerContext.hpp"
#include "Utils/DateUtils.hpp"
#include "Utils/VideoBuffer.hpp"
#include "WebsocketNotificatorController.hpp"
#include "nlohmann/json.hpp"
#include "observer/Domain/Notification/LocalNotifications.hpp"
#include "observer/Log/log.hpp"
#include "server_utils.hpp"
#include "uWebSockets/App.h"
#include "uWebSockets/MoveOnlyFunction.h"

#ifdef __linux__
#include <sys/sysinfo.h>
#include <unistd.h>
#endif

namespace Web::Controller {
    extern const std::unordered_map<std::string, const std::string> endpoints;

    inline size_t get_free_memory() {
#ifdef __linux__
        return get_phys_pages() * sysconf(_SC_PAGESIZE);
#elif _WIN32
        return std::numeric_limits<size_t>::max();
#endif
    }

    template <bool SSL>
    class NotificationController final
        : public Observer::INotificationEventSubscriber,
          public Observer::IEventValidatorSubscriber {
       public:
        NotificationController(uWS::App* app,
                               Web::DAL::INotificationRepository* nRepo,
                               Web::DAL::VideoBufferRepositoryNLDB* vbRepo,
                               Web::CL::NotificationCL* nCache,
                               Web::DAL::IConfigurationDAO* configurationDAO,
                               Web::ServerContext<SSL>* serverCtx);

        void OnOpenWebsocket(auto* ws,
                             const std::list<std::string_view>& paths);

        void OnCloseWebsocket(auto* ws, int /*code*/,
                              std::string_view /*message*/);

        void StreamNotification(auto* res, auto* req);

        void GetNotifications(auto* res, auto* req);

        void ReclaimTemporalBuffer(auto* res, auto* req);

        void GetVideoBufferOfNotificationGroup(auto* res, auto* req);

        void update(Observer::DTONotification ev) override;

        void update(
            Observer::EventDescriptor& event,
            std::shared_ptr<Observer::CameraEvent> rawCameraEvent) override;

       private:
        std::vector<Web::API::DTONotification> NotificationsToDTO(
            const std::vector<Domain::Notification>&);

        Web::API::DTONotification inline NotificationToDTO(
            const Domain::Notification&);

       private:
        Web::DAL::INotificationRepository* notificationRepository;
        Web::DAL::VideoBufferRepositoryNLDB* vbRepo;
        Web::CL::NotificationCL* notificationCache;
        Web::DAL::IConfigurationDAO* configurationDAO;
        Web::WebsocketNotificator<SSL> notificatorWS;
        Web::ServerContext<SSL>* serverCtx;

        std::vector<std::future<void>> pendingTasks;
    };

    template <bool SSL>
    NotificationController<SSL>::NotificationController(
        uWS::App* app, Web::DAL::INotificationRepository* pNotRepo,
        Web::DAL::VideoBufferRepositoryNLDB* pVBRepo,
        Web::CL::NotificationCL* pNotCache,
        Web::DAL::IConfigurationDAO* pConfigurationDAO,
        Web::ServerContext<SSL>* pServerCtx)
        : notificationRepository(pNotRepo),
          vbRepo(pVBRepo),
          notificationCache(pNotCache),
          configurationDAO(pConfigurationDAO),
          serverCtx(pServerCtx) {
        // http
        app->get(endpoints.at("notification-stream") + ":id",
                 [this](auto* res, auto* req) {
                     this->StreamNotification(res, req);
                 });

        app->get(
            endpoints.at("api-notifications"),
            [this](auto* res, auto* req) { this->GetNotifications(res, req); });

        app->post(endpoints.at("api-notification-buffer"),
                  [this](auto* res, auto* req) {
                      this->ReclaimTemporalBuffer(res, req);
                  });

        app->get(endpoints.at("api-notification-buffer"),
                 [this](auto* res, auto* req) {
                     this->GetVideoBufferOfNotificationGroup(res, req);
                 });

        // ws
        notificatorWS.Start();

        app->ws<PerSocketData>(
            endpoints.at("ws-notifications"),
            {.compression = uWS::CompressOptions::SHARED_COMPRESSOR,
             .open =
                 [this](auto* ws, const std::list<std::string_view>& paths) {
                     this->notificatorWS.AddClient(ws);
                 },
             .close =
                 [this](auto* ws, int /*code*/, std::string_view /*message*/) {
                     this->notificatorWS.RemoveClient(ws);
                 }});

#if BUILD_DEBUG

        app->post(endpoints.at("api-notifications"), [this](auto* res,
                                                            auto* req) {
            res->onAborted([]() {});

            std::string buffer;

            res->onData([this, res, buffer = std::move(buffer)](
                            std::string_view data, bool last) mutable {
                buffer.append(data.data(), data.length());

                if (last) {
                    Observer::DTONotification notification;
                    auto objs = std::make_shared<
                        std::vector<Observer::object_detected_t>>();
                    try {
                        // We read all the data
                        notification = nlohmann::json::parse(buffer);
                        notification.objectsDetected = objs;
                    } catch (...) {
                        notification = Observer::DTONotification(
                            1, "test", Observer::ENotificationType::TEXT,
                            "test", objs);
                    }

                    Web::API::DTONotification apiNotification =
                        NotificationToDTO(Domain::Notification(notification));

                    // notify all websocket subscribers about it
                    notificatorWS.update(apiNotification);

                    // this->update(notification);

                    OBSERVER_TRACE("Added notification via api post endpoint");

                    res->end("Notification added");
                }
            });
        });

#endif
    }

    template <bool SSL>
    void NotificationController<SSL>::StreamNotification(auto* res, auto* req) {
        std::string id(req->getParameter(0));
        std::string rangeHeader(req->getHeader("range"));

        if (this->notificationCache->Exists(id)) {
            std::string filename = this->notificationCache->GetFilename(id);

            FileStreamer::GetInstance().streamFile(res, filename, rangeHeader);
        } else {
            res->writeStatus(HTTP_404_NOT_FOUND);
            res->end();
        }
    }

    template <bool SSL>
    void NotificationController<SSL>::update(Observer::DTONotification ev) {
        /**
         * Observer notification ->  Domain Notification -> (Get camera id and
         * add it to repository and get id) -> set those id to domain
         * notification -> (simplify it so can be sent to client) ->
         * -> Web::API::Notification -> send it through the websocket
         */

        auto notification = Domain::Notification(ev);

        notification.configurationID =
            this->serverCtx->recognizeContext.running_config_id;

        try {
            auto camera = configurationDAO->FindCamera(
                serverCtx->recognizeContext.running_config_id, ev.cameraName);

            notification.camera.cameraID = camera["id"];
            notification.camera.name = camera["name"];
            notification.camera.uri = camera["url"];
        } catch (const std::exception& e) {
            OBSERVER_ERROR(
                "Unexpected error: camera not found with name {0} on "
                "configuration {1}",
                ev.cameraName, serverCtx->recognizeContext.running_config_id);
            throw std::runtime_error(
                std::string("Camera was not found while trying to add a new "
                            "notification. parent what(): ") +
                e.what());
        }

        std::string newID = notificationRepository->Add(notification);

        notification.notificationID = newID;

        Web::API::DTONotification apiNotification =
            NotificationToDTO(notification);

        // notify all websocket subscribers about it
        notificatorWS.update(apiNotification);
    }

    template <bool SSL>
    void NotificationController<SSL>::ReclaimTemporalBuffer(auto* res,
                                                            auto* req) {
        std::string groupID_s(req->getParameter(0));

        int groupID;
        try {
            groupID = std::stoi(groupID_s);
        } catch (...) {
            res->writeStatus(HTTP_400_BAD_REQUEST)
                ->endProblemJson(
                    (nlohmann::json {{"title", "invalid group id"}}).dump());
            return;
        }

        // Exists a temp notification buffer with groupID = :group_id?
        auto tempVB =
            notificationRepository->GetNotificationDebugVideo(groupID);

        if (!tempVB.has_value()) {
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->endProblemJson(
                    (nlohmann::json {{"title", "video buffer not found"}})
                        .dump());
            return;
        }

        // Was it already reclaimed?
        if (!tempVB->videoBufferID.empty()) {
            res->writeStatus(HTTP_200_OK)
                ->endJson(
                    (nlohmann::json {{"videoBufferID", tempVB->videoBufferID}})
                        .dump());
            return;
        }

        // does the file from temp notification buffer exists
        if (!std::filesystem::exists(tempVB->filePath)) {
            res->writeStatus(HTTP_400_BAD_REQUEST)
                ->endProblemJson(
                    (nlohmann::json {
                         {"title", "the buffer file is no longer available"}})
                        .dump());
            return;
        }

        // Create a new entry in the VideoBuffer collection with the data from
        // the stored raw video

        // but first we need to move the path to the video buffer folder
        std::filesystem::path toPath =
            // root is the media folder
            std::filesystem::path(
                ServerConfigurationProvider::Get().mediaFolder)
            // append video buffer folder
            / Constants::NOTIF_CAMERA_VIDEO_BUFFER_FOLDER;

        if (!std::filesystem::exists(toPath)) {
            std::filesystem::create_directories(toPath);
        }

        //  append current filename
        toPath = toPath / std::filesystem::path(tempVB->filePath).filename();

        std::filesystem::rename(tempVB->filePath, toPath);

        // TODO: Use a IRepository with DTO pls...
        std::string id =
            vbRepo->Add(nlohmann::json {{"path", toPath},
                                        {"fps", tempVB->fps},
                                        {"camera_id", tempVB->camera_id},
                                        {"duration", tempVB->duration},
                                        {"date_unix", time(0)},
                                        {"state", "with_buffer"}});

        // Modify the temp buffer entry and set the videoBufferID
        notificationRepository->UpdateNotificationDebugVideo(tempVB->id, id);

        res->writeStatus(HTTP_200_OK)
            ->endJson((nlohmann::json {{"videoBufferID", id}}).dump());
    }

    template <bool SSL>
    void NotificationController<SSL>::GetVideoBufferOfNotificationGroup(
        auto* res, auto* req) {
        std::string groupID_s(req->getParameter(0));

        int groupID;
        try {
            groupID = std::stoi(groupID_s);
        } catch (...) {
            res->writeStatus(HTTP_400_BAD_REQUEST)
                ->endProblemJson(
                    (nlohmann::json {{"title", "invalid group id"}}).dump());
            return;
        }

        auto tempVB =
            notificationRepository->GetNotificationDebugVideo(groupID);

        // Video buffer is not stored
        if (!tempVB.has_value()) {
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->endProblemJson(
                    (nlohmann::json {{"title", "video buffer not found"}})
                        .dump());
            return;
        }

        // Video buffer is stored but not yet reclaimed
        if (tempVB->videoBufferID.empty()) {
            res->writeStatus(HTTP_200_OK)
                ->endJson((nlohmann::json {{"reclaimed", false}}).dump());
            return;
        }

        res->writeStatus(HTTP_200_OK)
            ->writeHeader("Cache-Control", "max-age=604800")
            ->endJson(
                (nlohmann::json {{"reclaimed", true},
                                 {"videoBufferID", tempVB->videoBufferID}})
                    .dump());
    }

    template <bool SSL>
    void NotificationController<SSL>::GetNotifications(auto* res, auto* req) {
        std::string limitStr(req->getQuery("limit"));

        // expecting a datetime string as ISO_8601
        std::string beforeStr(req->getQuery("before"));
        std::string afterStr(req->getQuery("after"));

        // first page = 1
        std::string pageStr(req->getQuery("page"));

        constexpr int MAX_LIMIT = 100;

        int page;
        int limit = MAX_LIMIT;

        // parse limit
        if (!limitStr.empty()) {
            try {
                limit = std::stoi(limitStr);
            } catch (...) {
                OBSERVER_WARN(
                    "Requested notifications with an invalid limit: {0}",
                    limitStr);
            }

            limit = limit > MAX_LIMIT || limit < 0 ? MAX_LIMIT : limit;
        }

        // parse page
        try {
            page = std::stoi(pageStr);
        } catch (...) {
            page = 1;
        }

        std::vector<Domain::Notification> notifications;

        if (beforeStr.empty() && afterStr.empty()) {
            notifications = notificationRepository->GetAll(limit, false, page);
        } else {
            std::time_t before;
            std::time_t after;

            // parse the dates
            if (!beforeStr.empty()) {
                before = Utils::datetimeISO_8601ToTime(beforeStr.c_str());
            } else {
                before = std::numeric_limits<time_t>::max();
            }

            if (!afterStr.empty()) {
                after = Utils::datetimeISO_8601ToTime(afterStr.c_str());
            } else {
                after = 0;
            }

            notifications = notificationRepository->GetBetweenDates(
                after, before, limit, false, page);
        }

        nlohmann::json jsonNotifications = NotificationsToDTO(notifications);
        res->endJson(jsonNotifications.dump());
    }

    template <bool SSL>
    std::vector<Web::API::DTONotification>
    NotificationController<SSL>::NotificationsToDTO(
        const std::vector<Domain::Notification>& parseNots) {
        std::vector<Web::API::DTONotification> parsed(parseNots.size());

        for (int i = 0; i < parseNots.size(); i++) {
            parsed[i] = this->NotificationToDTO(parseNots[i]);
        }

        return parsed;
    }

    template <bool SSL>
    Web::API::DTONotification NotificationController<SSL>::NotificationToDTO(
        const Domain::Notification& parseNot) {
        Web::API::DTONotification parsed(parseNot);

        if (parseNot.type != "text") {
            // hide real filename but tell where to find the media
            parsed.content = endpoints.at("notification-stream") + parsed.id;
        }

        return parsed;
    }

    template <bool SSL>
    void NotificationController<SSL>::update(
        Observer::EventDescriptor& event,
        std::shared_ptr<Observer::CameraEvent> rawCameraEvent) {
        // Block the caller thread so we record the raw frames
        // In a period of time you might see more calls to this function that
        // notifications sent to the client, that is because the notification
        // sender has some minimum time between notifications.

        std::string cameraID;
        try {
            auto camera = this->configurationDAO->FindCamera(
                this->serverCtx->recognizeContext.running_config_id,
                event.GetCameraName());
            cameraID = camera["id"];
        } catch (...) {
            OBSERVER_WARN(
                "Did not saved the camera event buffer because we could not "
                "find the camera");
            return;
        }

        /* --- deep copy the frames if there is enough memory --- */
        auto& og_frames = rawCameraEvent->GetFrames();
        size_t total = 0;
        for (auto& frame : og_frames) {
            auto t = frame.GetInternalFrame();
            total += t.step[0] * t.rows;
        }
        // OBSERVER_TRACE("Trying allocating buffer size: {0} MB",
        //                total / 1024 / 1024);

        std::vector<Observer::Frame>* frames = &rawCameraEvent->GetFrames();
        bool allocated = false;

        if (total < get_free_memory()) {
            std::vector<Observer::Frame>* cpFrames =
                new std::vector<Observer::Frame>();
            cpFrames->resize(og_frames.size());
            for (int i = 0; i < og_frames.size(); i++) {
                og_frames[i].CopyTo((*cpFrames)[i]);
            }

            allocated = true;
            frames = cpFrames;
        }

        auto writeFrames = [&, cameraID, groupId = rawCameraEvent->GetGroupID(),
                            frameRate = rawCameraEvent->GetFrameRate()](
                               std::vector<Observer::Frame>* images) {
            const auto folder =
                std::filesystem::path(
                    ServerConfigurationProvider::Get().mediaFolder +
                    Constants::NOTIF_DEBUG_VIDEO_FOLDER)
                    .string();

            if (!std::filesystem::exists(folder)) {
                std::filesystem::create_directories(folder);
            }

            std::string storedBufferPath =
                folder + std::to_string(groupId) + "_temp_buffer.tiff";

            const double duration = images->size() / frameRate;
            Web::Utils::SaveBuffer(*images, storedBufferPath);

            notificationRepository->AddNotificationDebugVideo(  // <-- line 513
                Web::DTONotificationDebugVideo {
                    .filePath = storedBufferPath,
                    .groupID = groupId,
                    .videoBufferID = "",
                    .fps = static_cast<int>(frameRate),
                    .duration = duration,
                    .date_unix = time(0),
                    .camera_id = cameraID});
        };

        if (!allocated) {
            writeFrames(frames);
        } else {
            // convert to native
            pendingTasks.push_back(std::async(
                std::launch::async,
                [allocated, frames, cFunc = std::move(writeFrames)]() {
                    cFunc(frames);
                    if (allocated) {
                        delete frames;
                    }
                }));
        }
    }

}  // namespace Web::Controller