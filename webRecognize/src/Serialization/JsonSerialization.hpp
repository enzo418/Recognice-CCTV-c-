#pragma once

#include "../Domain/Notification.hpp"
#include "../Notifications/DTONotification.hpp"
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

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Web::Domain::Camera, id, name, uri);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Web::DTONotification, id, content,
                                       groupID, type, datetime, cameraID);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Web::Domain::Notification, id, content,
                                       groupID, type, datetime, camera);

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Observer::DTONotification, content,
                                       groupID, type);

    /* ------------------- OBSERVER STATUS ------------------ */
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ObserverStatusDTO, running, config_id);

}  // namespace nlohmann