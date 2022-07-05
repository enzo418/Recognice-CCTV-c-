#pragma once

#include <json/json.h>

#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <string_view>

#include "../vendor/json_dto/json_dto.hpp"
#include "DTO/AvailableConfigurationDTO.hpp"
#include "DTO/AvailableConfigurationsDTO.hpp"
#include "iostream"
#include "observer/Domain/Configuration/ConfigurationParser.hpp"
#include "observer/Log/log.hpp"
#include "server_types.hpp"
#include "vector"

extern const std::string HTTP_MULTIPART;
extern const std::string HTTP_FORM_URLENCODED;
extern const char* HTTP_404_NOT_FOUND;
extern const char* HTTP_301_MOVED_PERMANENTLY;

std::string GetJsonString(const std::string& key, const std::string& value);

/** Transform
 * { {"key1", "val1"}, {"key2", "val2"}, ...} =>
 * => {"key1": "val1", "key2": "val2", ...]
 *
 */

std::string GetJsonString(
    const std::vector<std::tuple<std::string_view, std::string_view, bool>>& v);
/*
void GetJsonKeyValue(std::ostringstream& ss, const char* key,
                     const std::string& value);

void GetJsonKeyValue(std::ostringstream& ss, const char* key,
                     const char* value);

void GetJsonKeyValue(std::ostringstream& ss, const char* key, bool value);

template <typename T>
void GetJsonKeyValue(std::ostringstream& ss, const char* key, T value) {
    ss << "\"" << key << "\": " << value;
}
/*
template <typename S>
std::string JsonString(std::ostringstream& ss, S kv) {
    GetJsonKeyValue(ss, kv.first, kv.second);

    ss << "}";

    return ss.str();
}
template <typename S, typename... Tail>
std::string JsonString(std::ostringstream& ss,
                       typename std::pair<std::string, S>& kv,
                       const Tail&... args) {
    ss << GetJsonKeyValue(kv.first, kv.second) << ",";
    return JsonString(ss, args...);
}

template <typename... Tail>
std::string JsonString(const Tail&... args) {
    std::ostringstream stream;
    stream << "{";
    return JsonString(stream, args...);
}*/

/**
 * @brief Formats a alert message
 * @param status status of the alert, ok or error
 * @param message message of the alert
 * @param trigger_query query that triggered the alert, should be the same as
 * the query received
 */
std::string GetErrorAlertReponse(const std::string& message,
                                 const std::string& extra = "");

std::string GetSuccessAlertReponse(const std::string& message,
                                   const std::string& extra = "");

// datetime format is %d_%m_%Y_%H_%M_%S, that's the same as dd_mm_yyyy_hh_mm_ss
void AppendNotification(Json::Value& root, const std::string& type,
                        const std::string& content, const std::string& group_id,
                        const std::string& datetime,
                        const std::string& directory);

void ReadNotificationsFile(const std::string& fn, Json::Value& target);

void WriteNotificationsFile(const std::string& fn, Json::Value& notifications,
                            Json::FastWriter& writter);

AvailableConfigurationsDTO GetAvailableConfigurations(
    const std::vector<std::string>& directoriesToSeach);

std::string GetRecognizeStateJson(const bool& recognize_running);
