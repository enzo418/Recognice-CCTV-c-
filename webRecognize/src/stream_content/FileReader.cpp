#include "FileReader.hpp"

time_t FileReader::lastModificationEpoch() {
    return std::chrono::system_clock::to_time_t(
        std::chrono::file_clock::to_sys(
            std::filesystem::last_write_time(fileName)
        )
    );
}

FileReader::FileReader(std::string fileName) : fileName(fileName) {
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

void FileReader::updateFileSize() {
    fin.seekg(0, fin.end);
    fileSize = fin.tellg();
}

// Wrap filesystem to support cache in a future
void FileReader::seek(std::streamoff start, std::ios_base::seekdir dir) {
    fin.seekg(start, dir);
}

// Wrap filesystem to support cache in a future
void FileReader::read(char *outdata, std::streamsize size) {
    // TODO: Check current cache from this position with this size...
    fin.read(outdata, size);
}

// Wrap filesystem to support cache in a future
bool FileReader::good() {
    return fin.good();
}

// Wrap filesystem to support cache in a future
void FileReader::close() {
    fin.close();
}

// Wrap filesystem to support cache in a future
void FileReader::clear() {
    fin.clear();
}

// Wrap filesystem to support cache in a future
std::ios_base::seekdir FileReader::beg() {
    return fin.beg;
}

void FileReader::reload() {
    fin.open(fileName, std::ios::binary);
    updateFileSize();
}

// Updates if changed and then return the file size
long FileReader::getFileSize() {
    time_t lastMod = lastModificationEpoch();
    if (std::difftime(lastMod, lastModification) >= 0) {
        updateFileSize();
        lastModification = lastMod;
    }

    return fileSize;
}

std::string FileReader::getFileName() {
    return fileName;
}

std::string FileReader::getLastModificationTime() {        
    struct tm time;
    gmtime_r(&lastModification, &time);

    char buffer[256];
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S %Z", &time);
    return static_cast<std::string>(buffer);
}