#include "server_utils.hpp"

#include "observer/Domain/Configuration/ConfigurationParser.hpp"

const std::string HTTP_MULTIPART = "multipart/form-data";
const std::string HTTP_FORM_URLENCODED = "application/x-www-form-urlencoded";
const char* HTTP_404_NOT_FOUND = "404 Not Found";
const char* HTTP_301_MOVED_PERMANENTLY = "301 Moved Permanently";

std::string GetJsonString(const std::string& key, const std::string& value) {
    return fmt::format("{{\"{0}\": {1}}}", key, value);
}

/*
void inline GetJsonKeyValue(std::ostringstream& ss, const char* key,
                            const std::string& value) {
    ss << "\"" << key << "\":\"" << value << "\"";
}*/

void GetJsonKeyValue(std::ostringstream& ss, const char* key,
                     const std::string& value) {
    ss << "\"" << key << "\":\"" << value << "\"";
}

void GetJsonKeyValue(std::ostringstream& ss, const char* key,
                     const char* value) {
    ss << "\"" << key << "\":\"" << value << "\"";
}

void GetJsonKeyValue(std::ostringstream& ss, const char* key, bool value) {
    ss << "\"" << key << "\": " << (value ? "true" : "false");
}

std::string GetJsonString(
    const std::vector<std::tuple<std::string_view, std::string_view, bool>>&
        v) {
    std::string res = "{";
    for (auto& [key, value, quote] : v) {
        if (quote) {
            res += fmt::format("\"{}\": \"{}\",", key, value);
        } else {
            res += fmt::format("\"{}\": {},", key, value);
        }
    }

    res.pop_back();  // pop last ,
    res += "}";

    return res;
}

/**
 * @brief Formats a alert message
 * @param status status of the alert, ok or error
 * @param message message of the alert
 * @param trigger_query query that triggered the alert, should be the same as
 * the query received
 */
std::string GetErrorAlertReponse(const std::string& message,
                                 const std::string& extra) {
    static const std::string st = "error";

    return GetJsonString({{"status", st, true},
                          {"data", message, false},
                          {"extra", extra, true}});
}

std::string GetSuccessAlertReponse(const std::string& message,
                                   const std::string& extra) {
    static const std::string st = "ok";

    return GetJsonString({{"status", st, true},
                          {"data", message, false},
                          {"extra", extra, true}});
}

AvailableConfigurationsDTO GetAvailableConfigurations(
    const std::vector<std::string>& directoriesToSeach) {
    AvailableConfigurationsDTO configsFiles;

    for (const auto& directory : directoriesToSeach) {
        if (std::filesystem::exists(directory)) {
            for (const auto& entry :
                 std::filesystem::directory_iterator(directory)) {
                std::string path = entry.path().generic_string();
                const auto ext = entry.path().extension();
                if (ext == ".json") {
                    AvailableConfigurationDTO avCfg;

                    // open configuration
                    try {
                        auto cfg = Observer::ConfigurationParser::
                            ConfigurationFromJsonFile(path);
                        avCfg.name = cfg.name;
                        avCfg.hash =
                            std::to_string(std::hash<std::string> {}(path));
                    } catch (...) {
                        OBSERVER_TRACE(
                            "File {0} is no a valid configuration but it's"
                            "in a configuration directory !",
                            path);
                        continue;
                    }

                    configsFiles.names.push_back(avCfg);
                }
            }
        }
    }

    return configsFiles;
}

std::string GetRecognizeStateJson(const bool& recognize_running) {
    return GetJsonString("recognize_state_changed",
                         recognize_running ? "true" : "false");
}
