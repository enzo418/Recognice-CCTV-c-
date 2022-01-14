#include "server_utils.hpp"

#include <sstream>

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

// datetime format is %d_%m_%Y_%H_%M_%S, that's the same as dd_mm_yyyy_hh_mm_ss
void AppendNotification(Json::Value& root, const std::string& type,
                        const std::string& content, const std::string& group_id,
                        const std::string& datetime,
                        const std::string& directory) {
    Json::Value pnt;
    pnt["type"] = type;
    pnt["content"] = content;
    pnt["group_id"] = group_id;
    pnt["datetime"] = datetime;
    pnt["directory"] = directory;
    root.append(pnt);
}

void ReadNotificationsFile(const std::string& fn, Json::Value& target) {
    if (std::filesystem::exists(fn)) {
        Json::Reader reader;

        std::ifstream file_source(fn, std::ifstream::binary);

        if (!reader.parse(file_source, target, false)) {
            std::cout << "couldn't read the notifications history: the file \""
                      << fn << "\" has a bad json format!\n";
        }

        file_source.close();
    }
}

void WriteNotificationsFile(const std::string& fn, Json::Value& notifications,
                            Json::FastWriter& writter) {
    std::ofstream file(fn, std::ifstream::binary);
    file << writter.write(notifications);
    file.flush();
    file.close();
}

std::string GetConfigurationsPaths(
    const std::vector<std::string>& directoriesToSeach) {
    std::string configsFiles = "[";

    size_t i = 0;
    for (const auto& directory : directoriesToSeach) {
        if (std::filesystem::exists(directory)) {
            for (const auto& entry :
                 std::filesystem::directory_iterator(directory)) {
                std::string path = entry.path().generic_string();
                if (entry.path().extension() == ".ini") {
                    if (i > 0) configsFiles += ",";

                    configsFiles += "\"" + path + "\"";
                    i++;
                }
            }
        }
    }

    configsFiles += "]";

    return configsFiles;
}

std::string GetConfigurationsPathsJson(
    const std::vector<std::string>& directoriesToSeach) {
    return GetJsonString("configuration_files",
                         GetConfigurationsPaths(directoriesToSeach));
}

std::string GetRecognizeStateJson(const bool& recognize_running) {
    return GetJsonString("recognize_state_changed",
                         recognize_running ? "true" : "false");
}
