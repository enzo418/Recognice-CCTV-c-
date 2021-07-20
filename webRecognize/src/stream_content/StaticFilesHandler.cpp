#include "StaticFilesHandler.hpp"

StaticFilesHandler::StaticFilesHandler() {}

bool StaticFilesHandler::fileExists(std::string path) {
    return std::filesystem::exists(path);
}

// Adds a file handler to the list    
void StaticFilesHandler::addFileHandler(std::string path) {
    // we assume that the file exists
    std::cout << "[" << path << "] => [" << path << "] File handler didn't exist. Adding it\n";
    filesReader[path] = new FileReader(path);
}

bool StaticFilesHandler::fileHandlerExists(const std::string& path) {
    return filesReader.find(path) != filesReader.end();
}

// Returns the FileReader of a path
FileReader* StaticFilesHandler::getFileHandler(const std::string& path) {
    return filesReader.find(path)->second;
}

void StaticFilesHandler::removeHandlerIfExists(const std::string& path) {
    auto it = filesReader.find(path);
    if (it != filesReader.end()) filesReader.erase(it);
}