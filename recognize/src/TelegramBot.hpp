#pragma once

#include <iostream>
#include <string>
#include <thread>

#include <curl/curl.h>
#include <json/json.h>

#include <chrono>

#include "types_configuration.hpp"
#include "ConfigurationFile.hpp"
#include "utils.hpp"

namespace TelegramBot { 
	std::string GetLastMessageFromBot(std::string& apiKey, std::string& result, std::time_t& unixTimeMs, const std::vector<std::string>& authUsers);

	void SendMessageToChat(const std::string& message, std::string& chatID, std::string& apiKey);

	void SendMediaToChat(const std::string& imagePath, const std::string& caption, std::string& chatID, std::string& apiKey, bool isAnimation = false);
}