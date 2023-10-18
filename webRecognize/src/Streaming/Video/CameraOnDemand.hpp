#pragma once

#include <atomic>
#include <chrono>
#include <mutex>
#include <optional>
#include <string_view>

#include "LiveViewExceptions.hpp"
#include "SocketData.hpp"
#include "Streaming/Video/SourceOnDemand.hpp"
#include "Streaming/Video/StreamWriter.hpp"
#include "observer/Domain/BufferedSource.hpp"
#include "observer/IFunctionality.hpp"
#include "observer/Implementation.hpp"
#include "observer/Log/log.hpp"
#include "observer/Semaphore.hpp"
#include "observer/Timer.hpp"
#include "observer/Utils/SpecialEnums.hpp"

namespace Web::Streaming::Video {
    class CameraOnDemand : public SourceOnDemand {
       public:
        CameraOnDemand(const std::string& pCameraUri, int quality);

        void InternalStart() override;
        void PostStop() override;

       protected:
        void OpenCamera();

       private:
        std::string cameraUri;
        Observer::BufferedSource source;
    };

}  // namespace Web::Streaming::Video