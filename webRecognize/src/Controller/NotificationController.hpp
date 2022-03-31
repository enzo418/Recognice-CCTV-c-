#pragma once

#include <list>
#include <memory>
#include <string>

#include "../../../recognize/Observer/src/Domain/Notification/LocalNotifications.hpp"
#include "../../uWebSockets/src/App.h"
#include "../../vendor/json_dto/json_dto.hpp"
#include "../CL/NotificationCL.hpp"
#include "../DAL/INotificationRepository.hpp"
#include "../Domain/Camera.hpp"
#include "../Notifications/WebsocketNotificator.hpp"
#include "../stream_content/FileStreamer.hpp"

namespace Web::Controller {
    extern const std::unordered_map<std::string, const std::string> endpoints;

    template <bool SSL>
    class NotificationController
        : public Observer::INotificationEventSubscriber {
       public:
        NotificationController(uWS::App* app,
                               Web::DAL::INotificationRepository* nRepo,
                               Web::CL::NotificationCL* nCache);

        void OnOpenWebsocket(auto* ws,
                             const std::list<std::string_view>& paths);

        void OnCloseWebsocket(auto* ws, int /*code*/,
                              std::string_view /*message*/);

        void StreamNotification(auto* res, auto* req);

        void GetNotifications(auto* res, auto* req);

        void update(Observer::DTONotification ev) override;

       private:
        std::vector<Web::DTONotification> NotificationsToDTO(
            std::vector<Domain::Notification>&);

        Web::DTONotification inline NotificationToDTO(Domain::Notification&);

       private:
        Web::DAL::INotificationRepository* notificationRepository;
        Web::CL::NotificationCL* notificationCache;

        Web::WebsocketNotificator<SSL> notificatorWS;
    };

    template <bool SSL>
    NotificationController<SSL>::NotificationController(
        uWS::App* app, Web::DAL::INotificationRepository* pNotRepo,
        Web::CL::NotificationCL* pNotCache)
        : notificationRepository(pNotRepo), notificationCache(pNotCache) {
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
        auto converted = Domain::Notification(ev);

        // add random camera id to test it
        converted.camera.id = "0";

        notificationRepository->Add(converted);

        auto copy = NotificationToDTO(converted);

        // notify all websocket subscribers about it
        notificatorWS.update(copy);
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

        auto notifications = notificationRepository->GetAll(limit);

        // add random camera id to test it
        for (auto&& notf : notifications) {
            notf.camera.id = "0";
        }

        auto msg = json_dto::to_json<std::vector<Web::DTONotification>>(
            NotificationsToDTO(notifications));

        res->endJson(msg);
    }

    template <bool SSL>
    std::vector<Web::DTONotification>
    NotificationController<SSL>::NotificationsToDTO(
        std::vector<Domain::Notification>& parseNots) {
        std::vector<Web::DTONotification> parsed(parseNots.size());

        for (int i = 0; i < parseNots.size(); i++) {
            parsed[i] = this->NotificationToDTO(parseNots[i]);
        }

        return parsed;
    }

    template <bool SSL>
    Web::DTONotification NotificationController<SSL>::NotificationToDTO(
        Domain::Notification& parseNot) {
        Web::DTONotification parsed(parseNot);

        if (parseNot.type != "text") {
            // hide real filename but tell where to find the media
            parsed.content = endpoints.at("notification-stream") + parsed.id;
        }

        return parsed;
    }
}  // namespace Web::Controller