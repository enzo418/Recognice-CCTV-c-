#include "ConfigurationParser.hpp"

#include <exception>
#include <opencv2/opencv.hpp>
#include <tuple>

#include "yaml-cpp/exceptions.h"
#include "yaml-cpp/mark.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/parse.h"
#include "yaml-cpp/node/type.h"

namespace Observer::ConfigurationParser {
    std::string GetCustomMarkErrorMessage(const YAML::Mark& mark) {
        if (mark.is_null()) return "";
        return fmt::format("at line {}, column {} = position {}", mark.line + 1,
                           mark.column + 1, mark.pos);
    }

    std::string GetCustomErrorMessage(const std::string& message,
                                      const YAML::Exception& ex) {
        return fmt::format("{} {}\n\t what (parser): {}", message,
                           GetCustomMarkErrorMessage(ex.mark), ex.what());
    }

    Configuration ParseYAML(const std::string& filePath) {
        YAML::Node node;
        Configuration cfg;

        try {
            node = YAML::LoadFile(filePath);
        } catch (const YAML::BadFile& ex) {
            OBSERVER_ERROR("Couldn't open the configuration file.");

            throw ConfigurationFileError();
        }

        try {
            cfg = node["configuration"].as<Observer::Configuration>();
        } catch (const YAML::KeyNotFound& ex) {
            OBSERVER_ERROR(GetCustomErrorMessage(
                fmt::format(
                    "Couldn't parse the configuration file, REQUIRED "
                    "key \"{}\" was not found on the object that starts",
                    ex.key),
                ex));

            throw MissingKey(ex.key);
        } catch (const YAML::InvalidNode& ex) {
            OBSERVER_ERROR(GetCustomErrorMessage(
                "Couldn't parse the configuration file, invalid node found",
                ex));

            throw ConfigurationFileError();
        } catch (const YAML::BadConversion& ex) {
            OBSERVER_ERROR(GetCustomErrorMessage(
                "Couldn't parse the configuration file, unexpected format",
                ex));

            throw WrongType(ex.mark.line, ex.mark.column, ex.mark.pos);
        } catch (const YAML::ParserException& ex) {
            OBSERVER_ERROR(GetCustomErrorMessage(
                "Couldn't parse the configuration file, parser error", ex));

            throw ConfigurationFileError();
        } catch (const std::exception& ex) {
            OBSERVER_ERROR("Couldn't parse the configuration file: {}",
                           ex.what());

            throw ConfigurationFileError();
        }

        return cfg;
    }

    void EmmitYAML(const std::string& filePath, const Configuration& cfg) {
        std::ofstream fs(filePath);

        YAML::Node out;
        out["configuration"] = cfg;
        fs << out;

        fs.close();
    }

    std::string NodeAsJson(YAML::Node& obj) {
        YAML::Emitter emitter;
        emitter << YAML::DoubleQuoted << YAML::Flow << YAML::BeginSeq << obj;
        return std::string(emitter.c_str() + 1);  // Strip leading [ character
    }

    std::string GetConfigurationAsJSON(const Configuration& cfg) {
        YAML::Node node;
        node["configuration"] = cfg;
        return NodeAsJson(node);
    }

    void EmmitJSON(const std::string& filePath, const Configuration& cfg) {
        std::ofstream jsonout(filePath);
        auto jsonstr = GetConfigurationAsJSON(cfg);
        jsonout << jsonstr;
        jsonout.close();
    }

    Configuration ParseJSON(const std::string& filePath) {
        return ParseYAML(filePath);
    }

    bool TrySetNodeValue(Object& obj, std::string* keys, int keysCount,
                         const std::string& value) {
        if (keysCount == 1 && obj[keys[0]]) {
            obj[keys[0]] = YAML::Load(value);
            return true;
        } else if (keysCount > 1 && obj[keys[0]]) {
            Object tmp = obj[keys[0]];
            return TrySetNodeValue(tmp, &keys[1], keysCount - 1, value);
        }

        return false;
    }

    YAML::Node TryGetNodeValue(const Object& obj, std::string* keys,
                               int keysCount) {
        if (keysCount == 1 && obj[keys[0]]) {
            return obj[keys[0]];
        } else if (keysCount > 1 && obj[keys[0]]) {
            // TODO: Check it because surely this is copying the whole object!
            Object tmp = obj[keys[0]];
            return TryGetNodeValue(tmp, &keys[1], keysCount - 1);
        }

        return YAML::Node(YAML::NodeType::Null);
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

    bool TrySetConfigurationFieldValue(Object& obj, std::string_view path) {
        std::string value;
        auto [keys, keys_count] = GetPathKeys(path, value);

        if (keys_count == -1) return false;

        return TrySetNodeValue(obj, keys.data(), keys_count, value);
    }

    bool TryGetConfigurationFieldValue(Object& obj, std::string_view path,
                                       YAML::Node& output) {
        std::string value;

        auto [keys, keys_count] = GetPathKeys(path, value);

        if (keys_count == -1) return false;

        output = TryGetNodeValue(obj, keys.data(), keys_count);

        return true;
    }

    bool ReadConfigurationObject(const std::string& cofiguration,
                                 Object& output) {
        try {
            output = YAML::Load(cofiguration);
        } catch (...) {
            return false;
        }

        return true;
    }

    bool ReadConfigurationObjectFromFile(const std::string& filePath,
                                         Object& output) {
        try {
            output = YAML::LoadFile(filePath);
        } catch (...) {
            return false;
        }

        return true;
    }
}  // namespace Observer::ConfigurationParser
