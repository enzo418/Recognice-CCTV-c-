#include <map>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <future>
#include <chrono>
#include <filesystem>

struct FileReader {
private:
    /* The cache we have in memory for this file */
    std::string cache;
    int cacheOffset;
    bool hasCache;

    long fileSize;
    std::string fileName;
    std::ifstream fin;

    std::time_t lastModification;
    
    time_t lastModificationEpoch() {
        return std::chrono::system_clock::to_time_t(
            std::chrono::file_clock::to_sys(
                std::filesystem::last_write_time(fileName)
            )
        );
    }
public:
    FileReader(std::string fileName) : fileName(fileName) {
        fin.open(fileName, std::ios::binary);

        // get fileSize
        updateFileSize();

        // std::cout << "File size is: " << fileSize << std::endl;

        // TODO: Since the file will be opened all the time check
        // if the last modification time is the same as the one
        // as when we opened it. Or the file will have
        // an incorrect size and we could potentially be 
        // trying to stream data that is not longer there.

        // TODO: Support cache

        // // cache up 10 mb!
        // cache.resize(10 * 1024 * 1024); // SHOULD CACHE THE FILE SIZE?¡¡!!!!

        // //std::cout << "Caching 10 MB at offset = " << 0 << std::endl;
        // fin.seekg(0, fin.beg);
        // fin.read(cache.data(), cache.length());
        // cacheOffset = 0;
        // hasCache = true;
    }

    void updateFileSize() {
        fin.seekg(0, fin.end);
        fileSize = fin.tellg();
    }

    // Wrap filesystem to support cache in a future
    void seek(std::streamoff start, std::ios_base::seekdir dir) {
        fin.seekg(start, dir);
    }

    // Wrap filesystem to support cache in a future
    void read(char *outdata, std::streamsize size) {
        // TODO: Check current cache from this position with this size...
        fin.read(outdata, size);
    }

    // Wrap filesystem to support cache in a future
    bool good() {
        return fin.good();
    }

    // Wrap filesystem to support cache in a future
    void close() {
        fin.close();
    }

    // Wrap filesystem to support cache in a future
    void clear() {
        fin.clear();
    }

    // Wrap filesystem to support cache in a future
    std::ios_base::seekdir beg() {
        return fin.beg;
    }

    void reload() {
        fin.open(fileName, std::ios::binary);
        updateFileSize();
    }

    // Updates if changed and then return the file size
    long getFileSize() {
        time_t lastMod = lastModificationEpoch();
        if (std::difftime(lastMod, lastModification) >= 0) {
            updateFileSize();
            lastModification = lastMod;
        }

        return fileSize;
    }

    std::string getFileName() {
        return fileName;
    }

    std::string getLastModificationTime() {        
        struct tm time;
        gmtime_r(&lastModification, &time);

        char buffer[256];
        strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %Z", &time);
        return static_cast<std::string>(buffer);
    }
};

struct StaticFilesHandler {
private:
    std::map<std::string, FileReader *> filesReader;
public:
    StaticFilesHandler() {}

    bool fileExists(std::string path) {
        return std::filesystem::exists(path);
    }

    // Adds a file handler to the list    
    void addFileHandler(std::string path) {
        // we assume that the file exists
        std::cout << "[" << path << "] => [" << path << "] File handler didn't exist. Adding it\n";
        filesReader[path] = new FileReader(path);
    }

    bool fileHandlerExists(const std::string& path) {
        return filesReader.find(path) != filesReader.end();
    }

    // Returns the FileReader of a path
    FileReader* getFileHandler(const std::string& path) {
        return filesReader.find(path)->second;
    }

    void removeHandlerIfExists(const std::string& path) {
        auto it = filesReader.find(path);
        if (it != filesReader.end()) filesReader.erase(it);
    }
};