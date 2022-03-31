#pragma once

#include "../../vendor/json_dto/json_dto.hpp"
#include "../Domain/Notification.hpp"
#include "../Notifications/DTONotification.hpp"

namespace json_dto {
    template <typename Json_Io>
    void json_io(Json_Io& io, Web::Domain::Notification& m) {
        io& json_dto::mandatory("id", m.id) &
            json_dto::mandatory("content", m.content) &
            json_dto::mandatory("group", m.groupID) &
            json_dto::mandatory("type", m.type) &
            json_dto::mandatory("date", m.datetime) &
            json_dto::mandatory("camera", m.camera);
    }

    template <typename Json_Io>
    void json_io(Json_Io& io, Web::DTONotification& m) {
        io& json_dto::mandatory("id", m.id) &
            json_dto::mandatory("content", m.content) &
            json_dto::mandatory("group", m.groupID) &
            json_dto::mandatory("type", m.type) &
            json_dto::mandatory("date", m.datetime) &
            json_dto::mandatory("cameraID", m.cameraID);
    }
} /* namespace json_dto */