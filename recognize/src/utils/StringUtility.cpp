#include "StringUtility.hpp"
namespace Observer::StringUtility {
    void StringToLower(std::string &str) {
        std::transform(str.begin(), str.end(), str.begin(),
                       [](unsigned char c) { return std::tolower(c); });
    }

    std::string GetStringBetweenDelimiter(const std::string& str, const std::string& start_delimiter, const std::string& stop_delimiter) {
        unsigned first = str.find(start_delimiter);
        unsigned last = str.find(stop_delimiter);
        return str.substr(first, last - first);
    }
}