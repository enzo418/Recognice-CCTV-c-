#include "FileStreamer.hpp"

FileStreamer::FileStreamer(std::string root) : root(root) {
    staticFiles = new StaticFilesHandler();
}

FileStreamer& FileStreamer::GetInstance() {
    OBSERVER_ASSERT(instance != nullptr,
                    "FileStreamer is not yet initialized!!");
    return *instance;
}

FileStreamer* FileStreamer::instance = nullptr;