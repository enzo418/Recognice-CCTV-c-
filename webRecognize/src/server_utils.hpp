#pragma once

#include "iostream"
#include "vector"

#include <fmt/core.h>
#include <fstream>
#include <filesystem>
#include "server_types.hpp"

extern const std::string HTTP_MULTIPART;
extern const std::string HTTP_FORM_URLENCODED;
extern const char *HTTP_404_NOT_FOUND;
extern const char *HTTP_301_MOVED_PERMANENTLY;

std::string GetJsonString(const std::string& key, const std::string& value);

/** Transform
 * { {"key1", "val1"}, {"key2", "val2"}, ...} => 
 * => {"key1": "val1", "key2": "val2", ...]
 * 
*/
std::string GetJsonString(const std::vector<std::pair<std::string, std::string>>& v);

std::string GetJsonString(const std::vector<std::pair<std::string, std::string>>& v, bool whitoutQuote);

/**
 * @brief Formats a alert message
 * @param status status of the alert, ok or error
 * @param message message of the alert
 * @param trigger_query query that triggered the alert, should be the same as the query received
 */
std::string GetAlertMessage(const AlertStatus& status, const std::string& message, const std::string& extra = "");

// datetime format is %d_%m_%Y_%H_%M_%S, that's the same as dd_mm_yyyy_hh_mm_ss
void AppendNotification (Json::Value& root, const std::string& type, const std::string& content, const std::string& group_id, const std::string& datetime);

void ReadNotificationsFile(const std::string& fn, Json::Value& target);

void WriteNotificationsFile(const std::string& fn, Json::Value& notifications, Json::FastWriter& writter);

std::string GetConfigurationsPaths(const std::vector<std::string>& directoriesToSeach);

std::string GetConfigurationsPathsJson(const std::vector<std::string>& directoriesToSeach);

std::string GetRecognizeStateJson(const bool& recognize_running);
