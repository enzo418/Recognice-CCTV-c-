#pragma once

#include <chrono> // for system_clock
#include <iomanip> // for put_time
#include <sstream> // ostringstream
#include <ctime> // for std::time_t

namespace Observer::SpecialFunctions
{
    // output: 29_01_1900_23_41_13
    std::string GetCurrentTime() {	
        std::chrono::time_point<std::chrono::system_clock> timepoint = std::chrono::system_clock::now();
        std::time_t time_now_t = std::chrono::system_clock::to_time_t(timepoint);
        std::tm* now_tm = localtime(&time_now_t);

        std::ostringstream ss;
        ss << std::put_time(now_tm, "%d_%m_%Y_%H_%M_%S");
        return ss.str();
    }

    std::string FormatNotificationTextString(std::string str, const std::string& name) {
        size_t pos = str.find("{N}");

        if (pos != std::string::npos) {
            return str.replace(pos, 3, name);
        }

        return str;
    }
} // namespace Observer
