#include "NotificationEndPoints.hpp"

#include <exception>
#include <string_view>

#include "../../../recognize/Observer/src/Log/log.hpp"
#include "Notifications.hpp"

namespace Web {
    namespace {
        void inline ParseGroupID(int& groupID, auto* req) {
            try {
                groupID = std::stoi((std::string)req->getQuery("group_id"));
            } catch (const std::exception& ex) {
                OBSERVER_ERROR("Couldn't parse group id from request.");
            }
        }

        void inline SendNotification(uWS::App& app, NotificationsContext& ctx,
                                     const Web::DTONotification& notification) {
            const auto content = NotificationToJson(notification);

            OBSERVER_TRACE("Sending notification: {}", content);

            app.publish(std::string_view(ctx.socketTopic), content,
                        uWS::OpCode::TEXT, true);
        }
    }  // namespace

    void SetNotificationsEndPoints(uWS::App& app, NotificationsContext& ctx) {
        app.get(ctx.textEndpoint,
                [&app, &ctx](auto* res, auto* req) {
                    std::string text(req->getQuery("text"));

                    int groupID {-1};
                    ParseGroupID(groupID, req);

                    Web::DTONotification notification;
                    notification.type = NOTIFICATIONS_MAP.at(
                        (int)Observer::ENotificationType::TEXT);
                    notification.datetime = "now";
                    notification.caption = text;
                    notification.groupID = groupID;

                    SendNotification(app, ctx, notification);
                    res->end();
                })
            .get(ctx.imageEndpoint,
                 [&app, &ctx](auto* res, auto* req) {
                     std::string text(req->getQuery("text"));
                     std::string imagePath(req->getQuery("image_path"));

                     int groupID {-1};
                     ParseGroupID(groupID, req);

                     Web::DTONotification notification;
                     notification.type = NOTIFICATIONS_MAP.at(
                         (int)Observer::ENotificationType::IMAGE);
                     notification.datetime = "now";
                     notification.caption = text;
                     notification.groupID = groupID;
                     notification.mediaPath = imagePath;

                     SendNotification(app, ctx, notification);
                     res->end();
                 })
            .get(ctx.videoEndpoint, [&app, &ctx](auto* res, auto* req) {
                std::string text(req->getQuery("text"));
                std::string videoPath(req->getQuery("video_path"));

                int groupID {-1};
                ParseGroupID(groupID, req);

                Web::DTONotification notification;
                notification.type = NOTIFICATIONS_MAP.at(
                    (int)Observer::ENotificationType::VIDEO);
                notification.datetime = "now";
                notification.caption = text;
                notification.groupID = groupID;
                notification.mediaPath = videoPath;

                SendNotification(app, ctx, notification);
                res->end();
            });
    }
}  // namespace Web