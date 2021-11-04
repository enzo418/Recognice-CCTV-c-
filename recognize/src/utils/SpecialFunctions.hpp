#pragma once

#include <chrono> // for system_clock
#include <iomanip> // for put_time
#include <sstream> // ostringstream
#include <ctime> // for std::time_t
#include <vector>

namespace Observer::SpecialFunctions
{
    // output: 29_01_1900_23_41_13
    std::string GetCurrentTime();

    std::string FormatNotificationTextString(std::string str, const std::string& name);

    /**
     * Returns a JSON string with all the pairs given
     * e.g.
     *  > func({{"url", "test"}, {"path", "c:/here"}})
     *  < {"url": "test","path": "c:/here"}
     * @param pairs all the key-value pairs to generate
     * @return
     */
    std::string JsonStringGenerator(const std::vector<std::pair<std::string, std::string>>& pairs);
} // namespace Observer
