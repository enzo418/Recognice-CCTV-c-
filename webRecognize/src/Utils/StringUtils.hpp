#pragma once
#include <algorithm>
#include <charconv>
#include <stdexcept>
#include <string>

namespace Web::StringUtils {

    inline std::string toLowercaseCopy(const std::string& str) {
        std::string ret = str;
        std::transform(ret.begin(), ret.end(), ret.begin(), ::tolower);
        return ret;
    }

    inline void toLowercase(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    }

    inline void replaceSubstring(std::string& str, const std::string& from,
                                 const std::string& to) {
        size_t start_pos = 0;
        while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
            str.replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }

    inline std::string decodeURL(const std::string_view& url) {
        std::string decoded;

        for (size_t i = 0; i < url.length(); ++i) {
            if (url[i] == '%') {
                if (i + 2 < url.length()) {
                    std::string_view hex = url.substr(i + 1, 2);
                    int value;
                    auto result = std::from_chars(
                        hex.data(), hex.data() + hex.size(), value, 16);

                    if (result.ec == std::errc::invalid_argument) {
                        decoded += url[i];
                        continue;
                    }

                    decoded += static_cast<char>(value);
                    i += 2;
                } else {
                    decoded += url[i];
                }
            } else if (url[i] == '+') {
                decoded += ' ';
            } else {
                decoded += url[i];
            }
        }

        return decoded;
    }

    inline std::string decodeURL(const std::string& str) {
        std::string ret;
        char ch;
        int i, ii, len = str.length();

        for (i = 0; i < len; i++) {
            if (str[i] != '%') {
                if (str[i] == '+')
                    ret += ' ';
                else
                    ret += str[i];
            } else if (i + 2 < len) {
                sscanf(str.substr(i + 1, 2).c_str(), "%x", &ii);
                ch = static_cast<char>(ii);
                ret += ch;
                i = i + 2;
            }
        }
        return ret;
    }
}  // namespace Web::StringUtils