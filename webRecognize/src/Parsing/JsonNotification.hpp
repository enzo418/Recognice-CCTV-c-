#include "../../vendor/json_dto/json_dto.hpp"
#include "../Domain/Notification.hpp"

namespace json_dto {
    template <typename Json_Io>
    void json_io(Json_Io& io, Web::Domain::Notification& m) {
        io& json_dto::mandatory("id", m.id) &
            json_dto::mandatory("content", m.content) &
            json_dto::mandatory("groupID", m.groupID) &
            json_dto::mandatory("type", m.type) &
            json_dto::mandatory("datetime", m.datetime);
    }
} /* namespace json_dto */