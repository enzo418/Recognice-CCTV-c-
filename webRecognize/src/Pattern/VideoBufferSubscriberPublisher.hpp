#pragma once

#include "observer/Pattern/ObserverBasics.hpp"

namespace BufferEventType {
    enum {
        CANCELED,        // data: error string
        BUFFER_READY,    // data: none
        DETECTION_DONE,  // data: none
        UPDATED          // data: json buffer
    };
}

typedef ISubscriber<std::string /*buffer id*/, int /*BufferEventType*/,
                    std::string /*data*/>
    VideoBufferSubscriber;

typedef Publisher<std::string /*buffer id*/, int /*BufferEventType*/,
                  std::string /*data*/>
    VideoBufferPublisher;