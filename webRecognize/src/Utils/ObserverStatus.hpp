#pragma once

#include <string>

#include "DTO/ObserverStatusDTO.hpp"
#include "Serialization/JsonSerialization.hpp"
#include "Server/RecognizeContext.hpp"
#include "nlohmann/json.hpp"

namespace Web::Utils {
    std::string inline GetStatusJsonString(RecognizeContext& recognizeContext) {
        bool running = recognizeContext.running;
        std::optional<std::string> cfg_id;
        if (running) cfg_id = recognizeContext.running_config_id;

        std::vector<Observer::CameraStatus> cameras =
            (running ? recognizeContext.observer->GetCamerasStatus()
                     : std::vector<Observer::CameraStatus> {});

        return nlohmann::json(Web::ObserverStatusDTO {.running = running,
                                                      .config_id = cfg_id,
                                                      .cameras = cameras})
            .dump();
    }
}  // namespace Web::Utils