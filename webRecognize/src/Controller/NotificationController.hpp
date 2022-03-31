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
        app->get("/stream/notification/:id", [this](auto* res, auto* req) {
            this->StreamNotification(res, req);
        });

        app->get("/api/notifications/", [this](auto* res, auto* req) {
            this->GetNotifications(res, req);
        });

        // ws
        notificatorWS.Start();

        app->ws<PerSocketData>(
            "/notifications",
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

        auto copy = Domain::Notification(converted);

        // don't tell the clients the real file location,
        // tell them where they can request to view it!
        copy.content = "/stream/notification/" + converted.id;

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

        auto msg =
            json_dto::to_json<std::vector<Domain::Notification>>(notifications);

        res->endJson(msg);
    }
}  // namespace Web::Controller