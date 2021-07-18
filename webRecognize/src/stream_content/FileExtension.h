#pragma once

#include <iostream>
#include <filesystem>
#include <unordered_map>
#include <string>

const std::unordered_map<std::string, std::string> contentTypes = {
    {"txt", "text/plain"},
    {"css", "text/css"},
    {"csv", "text/csv"},
    {"htm", "text/html"},
    {"html", "text/html"},
    {"xml", "text/xml"},
    {"js", "text/javascript"},
    {"xhtml", "application/xhtml+xml"},
    {"json", "application/json"},
    {"pdf", "application/pdf"},
    {"zip", "application/zip"},
    {"tar", "application/x-tar"},
    {"gif", "image/gif"},
    {"jpeg", "image/jpeg"},
    {"jpg", "image/jpeg"},
    {"tiff", "image/tiff"},
    {"tif", "image/tiff"},
    {"png", "image/png"},
    {"svg", "image/svg+xml"},
    {"ico", "image/x-icon"},
    {"swf", "application/x-shockwave-flash"},
    {"mp3", "audio/mpeg"},
    {"wav", "audio/x-wav"},
    {"ttf", "font/ttf"},
};

inline std::string getExtension(const std::string& path) {
    return std::filesystem::path(path).extension();
}

bool hasExtension(const std::string& url) {
    return !getExtension(url).empty();
}

const std::string& getContentType(const std::string& path) {
    auto it = contentTypes.find(getExtension(path));
    if (it != contentTypes.end()) {
        return it->second;
    }
    
    static const std::string defaultType("text/html");
    return defaultType;
}
