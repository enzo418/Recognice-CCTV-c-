#pragma once

#include <list>
#include <memory>
#include <string>

#include "../../../recognize/Observer/src/Domain/Notification/LocalNotifications.hpp"
#include "../../uWebSockets/src/App.h"
#include "../CL/NotificationCL.hpp"
#include "../DAL/INotificationRepository.hpp"
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

        notificationRepository->Add(converted);

        // don't tell the clients the real file location,
        // tell them where they can request to view it!
        ev.content = "/stream/notification/" + converted.id;

        // notify all websocket subscribers about it
        notificatorWS.update(ev);
    }
}  // namespace Web::Controller