#include <map>
#include <filesystem>
#include "FileReader.hpp"

struct StaticFilesHandler {
private:
    std::map<std::string, FileReader *> filesReader;
public:
    StaticFilesHandler();

    bool fileExists(std::string path);

    // Adds a file handler to the list    
    void addFileHandler(std::string path);

    bool fileHandlerExists(const std::string& path);

    // Returns the FileReader of a path
    FileReader* getFileHandler(const std::string& path);

    void removeHandlerIfExists(const std::string& path);
};