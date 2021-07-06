#pragma once

#include "iostream"
#include "vector"

#include <fmt/core.h>
#include <fstream>
#include <filesystem>
#include "server_types.hpp"

std::string GetJsonString(const std::string& key, const std::string& value) {
	return fmt::format("{{\"{0}\": {1}}}", key, value);
}

/** Transform
 * { {"key1", "val1"}, {"key2", "val2"}, ...} => 
 * => {"key1": "val1", "key2": "val2", ...]
 * 
*/
std::string GetJsonString(const std::vector<std::pair<std::string, std::string>>& v) {
	std::string res = "{";
	for (auto &&i : v) {
		res += fmt::format("\"{}\": \"{}\",", i.first, i.second);
	}
	
	res.pop_back(); // pop last ,
	res += "}";

	return res;
}

std::string GetJsonString(const std::vector<std::pair<std::string, std::string>>& v, bool whitoutQuote) {
	std::string res = "{";
	for (auto &&i : v) {
		res += fmt::format("\"{}\": {},", i.first, i.second);
	}
	
	res.pop_back(); // pop last ,
	res += "}";

	return res;
}

/**
 * @brief Formats a alert message
 * @param status status of the alert, ok or error
 * @param message message of the alert
 * @param trigger_query query that triggered the alert, should be the same as the query received
 */
std::string GetAlertMessage(const AlertStatus& status, const std::string& message, const std::string& trigger_query  = "", const std::string& extra = "") {
	std::string st = AlertStatus::OK == status ? "ok" : "error";

	return GetJsonString("request_reply", GetJsonString({{"status", st}, {"message", message}, {"trigger", trigger_query}, {"extra", extra}}));
}

// datetime format is %d_%m_%Y_%H_%M_%S, that's the same as dd_mm_yyyy_hh_mm_ss
void AppendNotification (Json::Value& root, const std::string& type, const std::string& content, const ulong& group_id, const std::string& datetime) {
    Json::Value pnt;
    pnt["type"] = type;
    pnt["content"] = content;
    pnt["group_id"] = (Json::LargestUInt)(group_id);
    pnt["datetime"] = datetime;
    root.append(pnt);
}

void ReadNotificationsFile(const std::string& fn, Json::Value& target) {
	if (std::filesystem::exists(fn)) {
        Json::Reader reader;

		std::ifstream file_source(fn, std::ifstream::binary);

        if (!reader.parse(file_source, target, false)) {
            std::cout << "couldn't read the notifications history: the file \"" << fn << "\" has a bad json format!\n";
        }
        
		file_source.close();
	}
}

void WriteNotificationsFile(const std::string& fn, Json::Value& notifications, Json::FastWriter& writter) {
	std::ofstream file(fn, std::ifstream::binary);
	file << writter.write(notifications);
    file.flush();
    file.close();
}

std::string GetConfigurationsPaths(const std::vector<std::string>& directoriesToSeach) {
	std::string configsFiles = "[";

	size_t i = 0;
	for (const auto& directory : directoriesToSeach) {
		if (std::filesystem::exists(directory)) {
			for (const auto & entry : std::filesystem::directory_iterator(directory)) {
				std::string path = entry.path().generic_string();
				if (entry.path().extension() == ".ini") {
					if(i > 0) configsFiles += ",";

					configsFiles += "\"" + path + "\"";
					i++;
				}
			}
		}
	}

	configsFiles += "]";
	
	return configsFiles;
}

std::string GetConfigurationsPathsJson(const std::vector<std::string>& directoriesToSeach) {
	return GetJsonString("configuration_files", GetConfigurationsPaths(directoriesToSeach));
}

std::string GetRecognizeStateJson(const bool& recognize_running) {
	return GetJsonString("recognize_state_changed", recognize_running ? "true" : "false");
}
