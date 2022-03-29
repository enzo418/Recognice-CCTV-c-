#include "NotificationCL.hpp"

namespace Web::CL {
    NotificationCL::NotificationCL(DAL::INotificationRepository* pRepo)
        : notificationRepository(pRepo),
          notificationCache(32, 10),
          notificationFilenameCache(40, 10) {}

    Domain::Notification NotificationCL::Get(const std::string& id) {
        Domain::Notification notif;

        if (!notificationCache.tryGet(id, notif)) {
            notif = notificationRepository->Get(id);
            notificationCache.insert(id, notif);
        }

        return notif;
    }

    bool NotificationCL::Exists(const std::string& id) {
        return notificationCache.contains(id) ||
               notificationRepository->Exists(id);
    }

    std::string NotificationCL::GetFilename(const std::string& id) {
        std::string filename;

        if (!notificationFilenameCache.tryGet(id, filename)) {
            filename = this->Get(id).content;
            notificationFilenameCache.insert(id, filename);
        }

        return filename;
    }

}  // namespace Web::CL