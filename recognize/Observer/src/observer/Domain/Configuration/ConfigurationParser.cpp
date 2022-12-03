#include "ConfigurationParser.hpp"

#include <exception>
#include <fstream>
#include <opencv2/opencv.hpp>
#include <tuple>

#include "observer/Domain/Configuration/Configuration.hpp"
#include "observer/Log/log.hpp"

namespace Observer::ConfigurationParser {
    void ConfigurationToJsonFile(const std::string& filePath,
                                 const Configuration& cfg) {
        nlohmann::json j = cfg;
        std::ofstream file(filePath);
        file << j;
    }

    Configuration ConfigurationFromJsonFile(const std::string& filePath) {
        nlohmann::json j;
        std::ifstream file(filePath);
        file >> j;
        Configuration cfg = j;
        return cfg;
    }

    json ConfigurationAsJSON(const Configuration& cfg) {
        json j = cfg;
        return j;
    }

    bool TrySetNodeValue(Object& obj, std::string* keys, int keysCount,
                         const std::string& value) {
        if (keysCount == 1 && obj.contains(keys[0])) {
            obj[keys[0]] = json::parse(value);
            return true;
        } else if (keysCount > 1 && !obj.is_array() && obj.contains(keys[0])) {
            return TrySetNodeValue(obj[keys[0]], &keys[1], keysCount - 1,
                                   value);
        } else if (keysCount > 1 && obj.is_array() &&
                   obj.size() > std::stoi(keys[0])) {
            return TrySetNodeValue(obj[std::stoi(keys[0])], &keys[1],
                                   keysCount - 1, value);
        }

        return false;
    }

    json TryGetNodeValue(const Object& obj, std::string* keys, int keysCount) {
        if (keysCount == 1 && obj.contains(keys[0])) {
            return obj[keys[0]];
        } else if (keysCount > 1 && obj.contains(keys[0])) {
            // TODO: Check it because surely this is copying the whole object!
            Object tmp = obj[keys[0]];
            return TryGetNodeValue(tmp, &keys[1], keysCount - 1);
        }

        return json {};
    }

    std::tuple<std::string_view, std::string_view> GetPathAndValue(
        std::string_view path) {
        auto pos = path.find("?to=");
        if (pos != std::string::npos) {
            std::string_view value = path.substr(pos + 4, path.length() - 1);

            // remove ?to=<value>
            path.remove_suffix(4 + (path.length() - (pos + 4)));

            if (value.empty()) {
                return std::make_tuple(path, value);
            }
        }

        return std::make_tuple("", "");
    }

    auto GetPathKeys(std::string_view path, std::string& value) {
        // each key represents a node on the configuration, so 50
        // seems reasonably high.
        std::array<std::string, 50> keys;
        int keys_count = 0;

        if (path.empty()) return std::make_tuple(keys, -1);

        auto pos = path.find("?to=");
        if (pos != std::string::npos) {
            value = path.substr(pos + 4, path.length() - 1);
            if (value.empty()) {
                return std::make_tuple(keys, -1);
            }

            // remove ?to=<value>
            path.remove_suffix(4 + (path.length() - (pos + 4)));
        }

        // split "/"
        while (path.length()) {
            auto pos = path.find("/");
            if (pos == 0) {
                // remove all / at the beggining
                path.remove_prefix(1);
            } else {
                // if didn't found / just add what is left as a key
                pos = pos == std::string::npos ? path.length() : pos;

                if (keys_count >= keys.max_size()) {
                    OBSERVER_WARN(
                        "UNEXPECTED high number of keys while setting Node "
                        "value");
                    return std::make_tuple(keys, -1);
                    ;
                }

                keys[keys_count++] = path.substr(0, pos);
                path.remove_prefix(pos);
            }
        }

        return std::make_tuple(std::move(keys), keys_count);
    }

    bool TrySetConfigurationFieldValue(Object& obj,
                                       std::string_view pathAndValue) {
        std::string value;
        auto [keys, keys_count] = GetPathKeys(pathAndValue, value);

        if (keys_count == -1) return false;

        return TrySetNodeValue(obj, keys.data(), keys_count, value);
    }

    json TryGetConfigurationFieldValue(Object& obj, std::string_view path) {
        std::string pathWithSlash(path);
        if (!path.starts_with("/")) pathWithSlash = "/" + pathWithSlash;

        return obj[nlohmann::json::json_pointer(pathWithSlash)];
    }
}  // namespace Observer::ConfigurationParser
