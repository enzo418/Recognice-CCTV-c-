#pragma once

#include <string>

struct PerSocketData {
    int id;
    std::string pathSubscribed;
};

struct VideoBufferSocketData {
    int id;
    std::string bufferID;
};
