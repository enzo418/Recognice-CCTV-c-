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

std::string GetLastMessageFromBot(std::string& apiKey, std::string& result);

void SendMessageToUser(const std::string& message, std::string& chatID, std::string& apiKey);