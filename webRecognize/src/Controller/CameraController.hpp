#pragma once

#include <chrono>
#include <future>

#include "ControllerUtils.hpp"
#include "DAL/IConfigurationDAO.hpp"
#include "Server/ServerContext.hpp"
#include "SocketData.hpp"
#include "Streaming/ws/WebsocketService.hpp"
#include "Utils/ObserverStatus.hpp"
#include "Utils/StringUtils.hpp"
#include "observer/Implementation.hpp"
#include "server_utils.hpp"
#include "uWebSockets/App.h"

namespace Web::Controller {
    template <bool SSL>
    class CameraController {
       public:
        CameraController(
            uWS::App* app, Web::DAL::IConfigurationDAO* configurationDAO,
            Web::ServerContext<SSL>* serverCtx,
            Web::Streaming::Ws::WebsocketService<SSL, PerSocketData>*
                wsService);

        void Get(auto* res, auto* req);
        void GetInfo(auto* res, auto* req);
        void GetFrame(auto* res, auto* req);

        /* --------------- Dynamic camera actions --------------- */
        // TODO: move to proper api documentation
        /**
         * @brief Change the camera type to view or notificator.
         *
         * NOTE: It will not write into the stored camera configuration.
         * NOTE: To disable it call snooze.
         * @param res
         * @param req
         * @param body  - json with the following members:
         *                  - type: string (disabled | view | notificator)
         *                      - disabled: the camera will be disabled
         *                      - view: the camera will be used only for live
         *                        view
         *                      - notificator: the camera will be used only for
         *                        view + notifications
         *                  - seconds: int (positive)
         *                      the camera will be changed to the specified type
         *                      for the specified amount of seconds. After that,
         *                      the camera will be changed back to it's original
         *                      type.
         */
        void TemporarilyChangeCameraType(auto* res, auto* req,
                                         const nlohmann::json& body);

        /**
         * @brief Change the camera type to disabled, view or notificator.
         *
         * NOTE: It will not write into the stored camera configuration.
         * @param res
         * @param req
         * @param body
         */
        void IndefinitelyChangeCameraType(auto* res, auto* req,
                                          const nlohmann::json& body);

        /* ----------------------- Stream ----------------------- */
        /**
         * @brief Request a live view of the camera. If successfully
         * generated the live view it returns a json with the ws_feed_path,
         * to which the client can request a WebSocket connection and we will
         * provide the images through it
         */
        void RequestStream(auto* res, auto* req);

       private:
        Web::DAL::IConfigurationDAO* configurationDAO;
        Web::ServerContext<SSL>* serverCtx;

        // url -> image
        std::unordered_map<std::string, std::vector<unsigned char>>
            cachedCameraImage;

        struct CachedCameraInfo {
            Observer::Size size;
            double fps;
            std::chrono::system_clock::time_point cachedAt;
        };
        std::unordered_map<std::string, CachedCameraInfo> cachedCameraInfo;

        Web::Streaming::Ws::WebsocketService<SSL, PerSocketData>*
            statusWsService;

        uWS::Loop* loop;

        std::vector<std::future<void>> asyncTasks;
    };

    template <bool SSL>
    CameraController<SSL>::CameraController(
        uWS::App* app, Web::DAL::IConfigurationDAO* pConfigurationDAO,
        Web::ServerContext<SSL>* pServerCtx,
        Web::Streaming::Ws::WebsocketService<SSL, PerSocketData>* wsService)
        : configurationDAO(pConfigurationDAO),
          serverCtx(pServerCtx),
          statusWsService(wsService) {
        app->get("/api/camera/:id",
                 [this](auto* res, auto* req) { this->Get(res, req); });

        app->get("/api/camera/:id/info",
                 [this](auto* res, auto* req) { this->GetInfo(res, req); });

        app->get("/api/camera/:id/frame",
                 [this](auto* res, auto* req) { this->GetFrame(res, req); });

        app->post("/api/camera/:name/type/temporarily",
                  [this](auto* res, auto* req) {
                      READ_JSON_BODY(res, req, TemporarilyChangeCameraType);
                  });

        app->post("/api/camera/:name/type/indefinitely",
                  [this](auto* res, auto* req) {
                      READ_JSON_BODY(res, req, IndefinitelyChangeCameraType);
                  });

        loop = uWS::Loop::get();
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
            return;
        }

        if (cachedCameraInfo.contains(uri)) {
            auto& info = cachedCameraInfo[uri];

            // if the info is cached and it's not older than 1 minute
            if (std::chrono::system_clock::now() - info.cachedAt <
                std::chrono::minutes(1)) {
                nlohmann::json response = {{"fps", info.fps},
                                           {"size", info.size}};

                res->endJson(response.dump());
                return;
            }
        }

        // don't block the main thread
        res->onAborted([]() {});

        asyncTasks.push_back(std::async(std::launch::async, [this, res, uri]() {
            Observer::VideoSource cap;
            cap.Open(uri);

            if (cap.isOpened()) {
                double fps = cap.GetFPS();
                Observer::Size size = cap.GetSize();

                nlohmann::json response = {{"fps", fps}, {"size", size}};

                cap.Close();

                cachedCameraInfo[uri] = {size, fps,
                                         std::chrono::system_clock::now()};

                loop->defer(
                    [res, response]() { res->endJson(response.dump()); });
            } else {
                nlohmann::json response = {{"title", "Camera not available"}};

                loop->defer([res, response]() {
                    res->writeStatus(HTTP_404_NOT_FOUND)
                        ->writeHeader("Cache-Control", "max-age=5")
                        ->endProblemJson(response.dump());
                });
            }
        }));
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
            return;
        }

        // don't block the main thread
        res->onAborted([]() {});

        asyncTasks.push_back(std::async(std::launch::async, [this, res, uri]() {
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

                    loop->defer([res, response]() {
                        res->writeStatus(HTTP_500_INTERNAL_SERVER_ERROR)
                            ->endProblemJson(response.dump());
                    });
                    return;
                }

                std::vector<unsigned char> buffer;
                frame.EncodeImage(".jpg", 90, buffer);

                cachedCameraImage[uri] = std::move(buffer);
            }

            auto& buffer = cachedCameraImage[uri];

            loop->defer([res, buffer]() {
                res->writeHeader("content-type", "image/jpeg")
                    ->end(
                        std::string_view((char*)buffer.data(), buffer.size()));
            });
        }));
    }

    template <bool SSL>
    void CameraController<SSL>::TemporarilyChangeCameraType(
        auto* res, auto* req, const nlohmann::json& body) {
        std::string camera_name(
            Web::StringUtils::decodeURL(req->getParameter(0)));

        if (camera_name.empty()) {
            res->writeStatus(HTTP_400_BAD_REQUEST)->end();
            return;
        }

        if (body.is_null() || !body.contains("type") ||
            !body["type"].is_string() || !body.contains("seconds") ||
            !body["seconds"].is_number() || body["seconds"] < 0) {
            res->writeStatus(HTTP_400_BAD_REQUEST)
                ->end(
                    "Expected json members: type (view | notificator), seconds "
                    "(positive integer)");
            return;
        }

        std::string type = body["type"];

        StringUtils::toLowercase(type);

        Observer::ECameraType cameraType =
            type == "view"       ? Observer::ECameraType::VIEW
            : type == "disabled" ? Observer::ECameraType::DISABLED
                                 : Observer::ECameraType::NOTIFICATOR;

        int seconds = body["seconds"];

        if (serverCtx->recognizeContext.observer->TemporarilyChangeCameraType(
                camera_name, seconds, cameraType)) {
            res->end(nlohmann::json(
                         serverCtx->recognizeContext.observer->GetCameraStatus(
                             camera_name))
                         .dump());

            auto statusString =
                Utils::GetStatusJsonString(this->serverCtx->recognizeContext);

            statusWsService->SendToClients(statusString.c_str(),
                                           statusString.size());
        } else {
            nlohmann::json response = {{"title", "Camera not found"}};

            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->endProblemJson(response.dump());
        }
    }

    template <bool SSL>
    void CameraController<SSL>::IndefinitelyChangeCameraType(
        auto* res, auto* req, const nlohmann::json& body) {
        std::string camera_name(
            Web::StringUtils::decodeURL(req->getParameter(0)));

        if (camera_name.empty()) {
            res->writeStatus(HTTP_400_BAD_REQUEST)->end();
            return;
        }

        if (body.is_null() || !body.contains("type") ||
            !body["type"].is_string()) {
            res->writeStatus(HTTP_400_BAD_REQUEST)
                ->end("Expected json members: type (view | notificator)");
            return;
        }

        std::string type = body["type"];

        StringUtils::toLowercase(type);

        Observer::ECameraType cameraType =
            type == "view"       ? Observer::ECameraType::VIEW
            : type == "disabled" ? Observer::ECameraType::DISABLED
                                 : Observer::ECameraType::NOTIFICATOR;

        if (serverCtx->recognizeContext.observer->IndefinitelyChangeCameraType(
                camera_name, cameraType)) {
            res->end(nlohmann::json(
                         serverCtx->recognizeContext.observer->GetCameraStatus(
                             camera_name))
                         .dump());

            auto statusString =
                Utils::GetStatusJsonString(this->serverCtx->recognizeContext);

            statusWsService->SendToClients(statusString.c_str(),
                                           statusString.size());
        } else {
            nlohmann::json response = {{"title", "Camera not found"}};

            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->endProblemJson(response.dump());
        }
    }
}  // namespace Web::Controller