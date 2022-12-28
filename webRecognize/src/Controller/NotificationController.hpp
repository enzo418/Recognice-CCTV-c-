#pragma once

#include <list>
#include <memory>
#include <stdexcept>
#include <string>

#include "../CL/NotificationCL.hpp"
#include "../DAL/INotificationRepository.hpp"
#include "../Domain/Camera.hpp"
#include "../stream_content/FileStreamer.hpp"
#include "DAL/ConfigurationDAO.hpp"
#include "DTO/DTONotification.hpp"
#include "Domain/Notification.hpp"
#include "Server/ServerContext.hpp"
#include "WebsocketNotificatorController.hpp"
#include "nlohmann/json.hpp"
#include "observer/Domain/Notification/LocalNotifications.hpp"
#include "observer/Log/log.hpp"
#include "uWebSockets/App.h"

namespace Web::Controller {
    extern const std::unordered_map<std::string, const std::string> endpoints;

    template <bool SSL>
    class NotificationController final
        : public Observer::INotificationEventSubscriber {
       public:
        NotificationController(uWS::App* app,
                               Web::DAL::INotificationRepository* nRepo,
                               Web::CL::NotificationCL* nCache,
                               Web::DAL::ConfigurationDAO* configurationDAO,
                               Web::ServerContext<SSL>* serverCtx);

        void OnOpenWebsocket(auto* ws,
                             const std::list<std::string_view>& paths);

        void OnCloseWebsocket(auto* ws, int /*code*/,
                              std::string_view /*message*/);

        void StreamNotification(auto* res, auto* req);

        void GetNotifications(auto* res, auto* req);

        void update(Observer::DTONotification ev) override;

       private:
        std::vector<Web::API::DTONotification> NotificationsToDTO(
            const std::vector<Domain::Notification>&);

        Web::API::DTONotification inline NotificationToDTO(
            const Domain::Notification&);

       private:
        Web::DAL::INotificationRepository* notificationRepository;
        Web::CL::NotificationCL* notificationCache;
        Web::DAL::ConfigurationDAO* configurationDAO;
        Web::WebsocketNotificator<SSL> notificatorWS;
        Web::ServerContext<SSL>* serverCtx;
    };

    template <bool SSL>
    NotificationController<SSL>::NotificationController(
        uWS::App* app, Web::DAL::INotificationRepository* pNotRepo,
        Web::CL::NotificationCL* pNotCache,
        Web::DAL::ConfigurationDAO* pConfigurationDAO,
        Web::ServerContext<SSL>* pServerCtx)
        : notificationRepository(pNotRepo),
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
                    // We read all the data
                    Observer::DTONotification notification =
                        nlohmann::json::parse(buffer);

                    this->update(notification);

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
    void NotificationController<SSL>::GetNotifications(auto* res, auto* req) {
        std::string limitStr(req->getQuery("limit"));

        constexpr int MAX_LIMIT = 100;

        int limit = MAX_LIMIT;

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

        std::vector<Domain::Notification> notifications =
            notificationRepository->GetAll(limit);

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
}  // namespace Web::Controller