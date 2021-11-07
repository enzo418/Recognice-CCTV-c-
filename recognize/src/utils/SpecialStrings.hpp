#pragma once
#include <iostream>
#include <string>
#include <algorithm>

namespace Observer::StringUtility {
    void StringToLower(std::string& str);

    std::string GetStringBetweenDelimiter(const std::string& str, const std::string& start_delimiter, const std::string& stop_delimiter);
};
