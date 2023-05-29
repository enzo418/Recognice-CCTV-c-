#pragma once

// #include "HttpStreamingService.hpp"
// #include "uWebSockets/App.h"
// namespace Web::Controller {
//     template <bool SSL>
//     class StreamingController {
//        public:
//         StreamingController(uWS::App* app);

//        private:
//         void StreamCamera(auto* res, auto* req);
//         void StreamSource(auto* res, auto* req);
//         void StreamObserver(auto* res, auto* req);
//         void StreamDetectionConfiguration(auto* res, auto* req);

//         private:
//         std::map<std::string, HttpStreamingService<SSL>> streamingServices;
//     };

//     template <bool SSL>
//     StreamingController<SSL>::StreamingController(uWS::App* app) {
//         app->get("/api/stream/camera/:id", [this](auto* res, auto* req) {
//             this->StreamCamera(res, req);
//         });

//         app->get("/api/stream/source", [this](auto* res, auto* req) {
//             this->StreamSource(res, req);
//         });

//         app->get("/api/stream/observer", [this](auto* res, auto* req) {
//             this->StreamObserver(res, req);
//         });

//         app->get("/api/stream/config/detection", [this](auto* res, auto* req)
//         {
//             this->StreamDetectionConfiguration(res, req);
//         });
//     }

// }  // namespace Web::Controller