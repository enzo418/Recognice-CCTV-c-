#pragma once

#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <sstream>

struct FileReader {
   public:
    FileReader(const std::string& fileName);

    void updateFileSize();

    // Wrap filesystem to support cache in a future
    void seek(std::streamoff start, std::ios_base::seekdir dir);

    // Wrap filesystem to support cache in a future
    void read(char* outdata, std::streamsize size);

    // Wrap filesystem to support cache in a future
    bool good();

    // Wrap filesystem to support cache in a future
    void close();

    // Wrap filesystem to support cache in a future
    void clear();

    // Wrap filesystem to support cache in a future
    std::ios_base::seekdir beg();

    void reload();

    // Updates if changed and then return the file size
    long getFileSize();

    std::string getFileName();

    std::string getLastModificationTime();

   private:
    /* The cache we have in memory for this file */
    std::string cache;
    int cacheOffset;
    bool hasCache;

    long fileSize;
    std::string fileName;
    std::ifstream fin;

    std::time_t lastModification;

    time_t lastModificationEpoch();
};