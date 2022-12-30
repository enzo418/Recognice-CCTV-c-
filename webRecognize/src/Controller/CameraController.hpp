#pragma once

#include "DAL/ConfigurationDAO.hpp"
#include "observer/Implementation.hpp"
#include "server_utils.hpp"
#include "uWebSockets/App.h"

namespace Web::Controller {
    template <bool SSL>
    class CameraController {
       public:
        CameraController(uWS::App* app,
                         Web::DAL::ConfigurationDAO* configurationDAO);

        void Get(auto* res, auto* req);
        void GetInfo(auto* res, auto* req);
        void GetFrame(auto* res, auto* req);

       private:
        Web::DAL::ConfigurationDAO* configurationDAO;

        // url -> image
        std::unordered_map<std::string, std::vector<unsigned char>>
            cachedCameraImage;
    };

    template <bool SSL>
    CameraController<SSL>::CameraController(
        uWS::App* app, Web::DAL::ConfigurationDAO* pConfigurationDAO)
        : configurationDAO(pConfigurationDAO) {
        app->get("/api/camera/:id",
                 [this](auto* res, auto* req) { this->Get(res, req); });

        app->get("/api/camera/:id/info",
                 [this](auto* res, auto* req) { this->GetInfo(res, req); });

        app->get("/api/camera/:id/frame",
                 [this](auto* res, auto* req) { this->GetFrame(res, req); });
    }

    template <bool SSL>
    void CameraController<SSL>::Get(auto* res, auto* req) {
        auto id = std::string(req->getParameter(0));

        try {
            nldb::json camera = configurationDAO->GetCamera(id);

            //  TODO: We need to respond only with {name, id, url}
            //  for that we should use camera repository
            res->endJson(camera.dump());
        } catch (const std::exception& e) {
            nlohmann::json response = {{"title", "camera not found"}};
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->endProblemJson(response.dump());
        }
    }

    template <bool SSL>
    void CameraController<SSL>::GetInfo(auto* res, auto* req) {
        std::string uri;

        // then try to get it from the database with:
        std::string camera_id(req->getParameter(0));

        if (camera_id.empty()) {
            res->writeStatus(HTTP_400_BAD_REQUEST)->end();
            return;
        }

        try {
            auto camera = configurationDAO->GetCamera(camera_id);
            uri = camera["url"];
        } catch (const std::exception& e) {
            nlohmann::json response = {{"title", "Camera not found"}};

            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->endProblemJson(response.dump());
        }

        Observer::VideoSource cap;
        cap.Open(uri);

        if (cap.isOpened()) {
            double fps = cap.GetFPS();
            Observer::Size size = cap.GetSize();

            nlohmann::json response = {{"fps", fps}, {"size", size}};

            res->endJson(response.dump());

            cap.Close();
        } else {
            nlohmann::json response = {{"title", "Camera not avilable"}};

            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->endProblemJson(response.dump());
        }
    }

    template <bool SSL>
    void CameraController<SSL>::GetFrame(auto* res, auto* req) {
        std::string uri;

        // then try to get it from the database with:
        std::string camera_id(req->getParameter(0));

        if (camera_id.empty()) {
            res->writeStatus(HTTP_400_BAD_REQUEST)->end();
            return;
        }

        try {
            auto camera = configurationDAO->GetCamera(camera_id);
            uri = camera["url"];
        } catch (const std::exception& e) {
            nlohmann::json response = {{"title", "Camera not found"}};

            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->endProblemJson(response.dump());
        }

        if (!cachedCameraImage.contains(uri)) {
            Observer::VideoSource cap;
            Observer::Frame frame;

            cap.Open(uri);
            cap.GetNextFrame(frame);

            int tries = 100;
            while (frame.IsEmpty() && --tries) cap.GetNextFrame(frame);

            if (tries <= 0) {
                nlohmann::json response = {
                    {"title", "Camera didn't return any valid frames"}};

                res->writeStatus(HTTP_500_INTERNAL_SERVER_ERROR)
                    ->endProblemJson(response.dump());
                return;
            }

            std::vector<unsigned char> buffer;
            frame.EncodeImage(".jpg", 90, buffer);

            cachedCameraImage[uri] = std::move(buffer);
        }

        auto& buffer = cachedCameraImage[uri];
        res->writeHeader("content-type", "image/jpeg")
            ->end(std::string_view((char*)buffer.data(), buffer.size()));
    }

}  // namespace Web::Controller