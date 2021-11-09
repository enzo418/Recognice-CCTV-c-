#include "LocalWebNotifications.hpp"

#include <utility>

namespace Observer {
    LocalWebNotifications::LocalWebNotifications(std::string pRestServerUrl)
        : restServerUrl(std::move(pRestServerUrl)) {}

}  // namespace Observer
