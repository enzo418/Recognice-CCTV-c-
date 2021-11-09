#pragma once
#include <algorithm>
#include <iostream>
#include <string>

namespace Observer::StringUtility {
    void StringToLower(std::string& str);

    std::string GetStringBetweenDelimiter(const std::string& str,
                                          const std::string& start_delimiter,
                                          const std::string& stop_delimiter);
};  // namespace Observer::StringUtility
