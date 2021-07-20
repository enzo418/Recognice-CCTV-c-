#pragma once

#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <string>

extern const std::unordered_map<std::string, std::string> contentTypes;

inline std::string getExtension(const std::string& path);

bool hasExtension(const std::string& url);

const std::string& getContentType(const std::string& path);
