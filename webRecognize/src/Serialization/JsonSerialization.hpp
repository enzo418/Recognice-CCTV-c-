#pragma once

#include "../Domain/Notification.hpp"
#include "DTO/DTOBlob.hpp"
#include "DTO/DTONotification.hpp"
#include "DTO/ObserverStatusDTO.hpp"
#include "nlohmann/json.hpp"
#include "observer/Domain/Configuration/NLHJSONConfiguration.hpp"

namespace nlohmann {
    /* -------------------- STD::OPTIONAL ------------------- */
    // why? https://github.com/nlohmann/json/pull/2117
    template <class T>
    void to_json(nlohmann::json& j, const std::optional<T>& v) {
        if (v.has_value())
            j = *v;
        else
            j = nullptr;
    }

    template <class T>
    void from_json(const nlohmann::json& j, std::optional<T>& v) {
        if (j.is_null())
            v = std::nullopt;
        else
            v = j.get<T>();
    }

    // NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Web::Domain::Camera, cameraID, name,
    //                                    uri);

    void inline to_json(json& j, const Web::Domain::Camera& p) {
        j = json {{"cameraID", p.cameraID}, {"name", p.name}, {"uri", p.uri}};
    }

    void inline from_json(const json& j, Web::Domain::Camera& p) {
        // all as optional
        if (j.contains("cameraID")) j.at("cameraID").get_to(p.cameraID);
        if (j.contains("name")) j.at("name").get_to(p.name);
        if (j.contains("uri")) j.at("uri").get_to(p.uri);
    }

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(
        Web::API::DTONotification::DTONotificationCamera, id, name);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Web::API::DTONotification, id, content,
                                       groupID, type, datetime, camera);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Observer::DTONotification, content,
                                       groupID, type);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Web::Domain::Notification,
                                       notificationID, content, groupID, type,
                                       datetime, camera);

    /* ------------------- OBSERVER STATUS ------------------ */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Web::ObserverStatusDTO, running,
                                       config_id);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Web::DTOBlob, first_appearance,
                                       last_appearance, rects, internal_id);

    // void inline to_json(json& j, const Web::DTONotificationWithId& p) {
    //     j = json::parse(p.notification);
    //     j = j["id"] = p.id;
    // }

    // void inline from_json(const json& j, Web::DTONotificationWithId& p) {
    //     j.contains("_id") ? j.at("_id").get_to(p.id) :
    //     j.at("id").get_to(p.id); j.get_to(p.notification);
    // }

}  // namespace nlohmann