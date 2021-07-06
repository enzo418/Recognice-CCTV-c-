#define CATCH_CONFIG_MAIN
#include "catch.hpp"

#include <iostream>
#include <vector>

#include "../server_types.hpp"
#include "../server_utils.hpp"

TEST_CASE("Server") {
    SECTION("persistent notifications read and write") {
        const std::string& test_notification_file = "test_persistent_notification.json";
        Json::FastWriter writer;
        Json::Value notifications;
        
        // add values

        AppendNotification(notifications, "video", "../test/test.mp4", 1, "04_07_2021_20_40_00");
        AppendNotification(notifications, "gif", "../test/test.gif", 1, "04_07_2021_20_41_00");
        AppendNotification(notifications, "image", "../test/test.jpg", 2, "04_07_2021_20_42_00");
        AppendNotification(notifications, "text", "text asd", 3, "04_07_2021_20_43_00");

        // write and read several times

        for (size_t i = 1; i <= 3; i++) {
            WriteNotificationsFile(test_notification_file, std::ref(notifications), writer);

            notifications.clear();

            ReadNotificationsFile(test_notification_file, std::ref(notifications));            
        }

        // test values readed

        REQUIRE(notifications.size() == 4);
        REQUIRE(notifications[0]["type"] == "video");
        REQUIRE(notifications[1]["type"] == "gif");
        REQUIRE(notifications[2]["type"] == "image");
        REQUIRE(notifications[3]["type"] == "text");
        
        REQUIRE(notifications[0]["content"] == "../test/test.mp4");
        REQUIRE(notifications[0]["group_id"] == 1);
        REQUIRE(notifications[0]["datetime"] == "04_07_2021_20_40_00");
        
        REQUIRE(notifications[3]["group_id"] == 3);
    }
}
