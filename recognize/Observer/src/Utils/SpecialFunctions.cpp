#include "SpecialFunctions.hpp"

namespace Observer::SpecialFunctions {
    // output: 29_01_1900_23_41_13
    std::string GetCurrentTime(const std::string& format) {
        std::chrono::time_point<std::chrono::system_clock> timepoint =
            std::chrono::system_clock::now();
        std::time_t time_now_t =
            std::chrono::system_clock::to_time_t(timepoint);
        std::tm* now_tm = localtime(&time_now_t);

        std::ostringstream ss;
        ss << std::put_time(now_tm, format.c_str());
        return ss.str();
    }

    std::string FormatNotificationTextString(std::string str,
                                             const std::string& name) {
        size_t pos = str.find("{N}");

        if (pos != std::string::npos) {
            return str.replace(pos, 3, name);
        }

        return str;
    }

    /**
     * Returns a JSON string with all the pairs given
     * e.g.
     *  > func({{"url", "test"}, {"path", "c:/here"}})
     *  < {"url": "test","path": "c:/here"}
     * @param pairs all the key-value pairs to generate
     * @return
     */
    std::string JsonStringGenerator(
        const std::vector<std::pair<std::string, std::string>>& pairs) {
        std::string res = "{";

        for (auto&& i : pairs) {
            res += "\"" + i.first + "\": \"" + i.second + "\",";
        }

        res.pop_back();  // pop last ,
        res += "}";

        return res;
    }
}  // namespace Observer::SpecialFunctions
