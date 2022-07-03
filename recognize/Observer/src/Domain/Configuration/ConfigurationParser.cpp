#include "ConfigurationParser.hpp"

#include <exception>
#include <opencv2/opencv.hpp>

#include "yaml-cpp/exceptions.h"
#include "yaml-cpp/node/node.h"
#include "yaml-cpp/node/parse.h"

namespace Observer::ConfigurationParser {
    Configuration ParseYAML(const std::string& filePath) {
        YAML::Node node;
        Configuration cfg;

        try {
            node = YAML::LoadFile(filePath);
        } catch (const YAML::BadFile& ex) {
            OBSERVER_ERROR("Couldn't open the configuration file.");

            // Todo throw custom ex

            throw std::exception();
        }

        try {
            cfg = node["configuration"].as<Observer::Configuration>();
        } catch (YAML::KeyNotFound ex) {
            OBSERVER_ERROR(
                "Couldn't parse the configuration file, ERROR: Key not found ",
                ex.msg);
        } catch (YAML::InvalidNode ex) {
            std::string message = "";
            if (!ex.mark.is_null()) {
                message = "Line: " + std::to_string(ex.mark.line) +
                          " Pos: " + std::to_string(ex.mark.pos);
            }
            message += " - " + ex.msg;

            // Log
            OBSERVER_ERROR("Couldn't parse the configuration file: {}",
                           message);

            auto key = Observer::StringUtility::GetStringBetweenDelimiter(
                ex.msg, "\"", "\"");

            throw MissingKey(key.c_str());
        }
        /*        catch (const YAML::TypedBadConversion &ex) {
                    std::cout << "Couldn't parse the configuration file: " <<
           std::endl;

                    if (!ex.mark.is_null()) {
                        std::cout
                                << "\n\t Line: " << ex.mark.line << "\n\t Pos: "
           << ex.mark.pos
                                << std::endl;
                    }

                    std::cout << "\tError: " << ex.msg << std::endl;

                    throw WrongType(ex.mark.line, ex.mark.column, ex.mark.pos);
                } */
        catch (const YAML::ParserException& ex) {
            std::string message = "";
            if (!ex.mark.is_null()) {
                message = "Line: " + std::to_string(ex.mark.line) +
                          " Pos: " + std::to_string(ex.mark.pos);
            }
            message += " - " + ex.msg;

            // Log
            OBSERVER_ERROR("Couldn't parse the configuration file: {}",
                           message);
        } catch (const std::exception& ex) {
            OBSERVER_ERROR("Couldn't parse the configuration file: {}",
                           ex.what());
            // TODO: This is soooooooo bad... return false or something...
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

    std::string GetConfigurationAsJSON(const Configuration& cfg) {
        YAML::Node node;
        node["configuration"] = cfg;
        YAML::Emitter emitter;
        emitter << YAML::DoubleQuoted << YAML::Flow << YAML::BeginSeq << node;
        return std::string(emitter.c_str() + 1);  // Strip leading [ character
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

    bool TrySetConfigurationFieldValue(Object& obj, std::string_view path) {
        // each key represents a node on the configuration, so 50
        // seems reasonably high.
        std::array<std::string, 50> keys;
        size_t keys_count = 0;

        std::string value;

        if (path.empty()) return false;

        auto pos = path.find("?to=");
        if (pos != std::string::npos) {
            value = path.substr(pos + 4, path.length() - 1);
            if (value.empty()) {
                return false;
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
                    return false;
                }

                keys[keys_count++] = path.substr(0, pos);
                path.remove_prefix(pos);
            }
        }

        return TrySetNodeValue(obj, keys.data(), keys_count, value);
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
