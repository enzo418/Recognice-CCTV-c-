#include "DAL/ConfigurationDAO.hpp"
#include "DAL/NoLiteDB/VideoBufferRepositoryNLDB.hpp"
#include "VideoBufferTasksManager.hpp"
#include "server_utils.hpp"
#include "stream_content/FileStreamer.hpp"
#include "uWebSockets/App.h"

namespace Web::Controller {
    template <bool SSL>
    class VideoBufferController {
       public:
        VideoBufferController(
            uWS::App* app, VideoBufferTasksManager* videoBufferTasksManager,
            Web::DAL::VideoBufferRepositoryNLDB* videoBufferRepository,
            Web::DAL::ConfigurationDAO* configurationDAO);

        void CreateBuffer(auto* res, auto* req);
        void GetAllBuffer(auto* res, auto* req);
        void DeleteBuffer(auto* res, auto* req);
        void SteamBuffer(auto* res, auto* req);

       private:
        Web::DAL::VideoBufferRepositoryNLDB* videoBufferRepository;
        Web::DAL::ConfigurationDAO* configurationDAO;
        VideoBufferTasksManager* videoBufferTasksManager;
    };

    template <bool SSL>
    VideoBufferController<SSL>::VideoBufferController(
        uWS::App* app, VideoBufferTasksManager* pVideoBufferTasksManager,
        Web::DAL::VideoBufferRepositoryNLDB* pVideoBufferRepository,
        Web::DAL::ConfigurationDAO* pConfigurationDAO)
        : videoBufferRepository(pVideoBufferRepository),
          configurationDAO(pConfigurationDAO),
          videoBufferTasksManager(pVideoBufferTasksManager) {
        app->put("/api/buffer/", [this](auto* res, auto* req) {
            this->CreateBuffer(res, req);
        });

        app->get("/api/buffer/", [this](auto* res, auto* req) {
            this->GetAllBuffer(res, req);
        });

        app->del("/api/buffer/:id", [this](auto* res, auto* req) {
            this->DeleteBuffer(res, req);
        });

        app->get("/stream/buffer/:id",
                 [this](auto* res, auto* req) { this->SteamBuffer(res, req); });
    }

    template <bool SSL>
    void VideoBufferController<SSL>::CreateBuffer(auto* res, auto* req) {
        /**
         * {
         *      duration <double>: buffer duration in seconds
         *
         *      delay <double>: seconds to wait before starting the
         *
         *      resize <{width, height}>:
         *          size to which resize the frames, set to 0,0 to
         * use the camera resolution
         *
         *      camera_id <string>:
         *          Will use this camera configuration url to get the
         *          buffer
         * }
         *
         */
        std::string buffer;

        // get it before we attach the reader
        auto ct = std::string(req->getHeader("content-type"));

        res->onAborted([]() {});

        res->onData([this, res, req, ct, buffer = std::move(buffer)](
                        std::string_view data, bool last) mutable {
            buffer.append(data.data(), data.length());

            if (last) {
                if (ct != "application/json") {
                    res->writeStatus(HTTP_400_BAD_REQUEST)
                        ->end("Expected a json body");
                    return;
                }

                nldb::json bufferAsJson;
                try {
                    bufferAsJson = nlohmann::json::parse(buffer);
                } catch (const std::exception& e) {
                    res->writeStatus(HTTP_400_BAD_REQUEST)
                        ->endProblemJson(
                            (nlohmann::json {
                                 {"title", "body isn't a valid json"}})
                                .dump());
                    return;
                }

                // TODO: this->ensureJson(duration, Type::Number)

                double duration;

                try {
                    duration = bufferAsJson["duration"].get<double>();
                } catch (const std::exception& e) {
                    res->writeStatus(HTTP_400_BAD_REQUEST)
                        ->endProblemJson(
                            (nlohmann::json {{"title",
                                              "required json "
                                              "numeric field 'duration'"}})
                                .dump());
                    return;
                }

                double delay;

                try {
                    delay = bufferAsJson["delay"].get<double>();
                } catch (const std::exception& e) {
                    res->writeStatus(HTTP_400_BAD_REQUEST)
                        ->endProblemJson(
                            (nlohmann::json {{"title",
                                              "required json "
                                              "numeric field 'delay'"}})
                                .dump());
                    return;
                }

                Observer::Size resize;

                try {
                    resize = bufferAsJson["resize"].get<Observer::Size>();
                } catch (const std::exception& e) {
                    res->writeStatus(HTTP_400_BAD_REQUEST)
                        ->endProblemJson(
                            (nlohmann::json {{"title",
                                              "required json "
                                              "field 'resize' of type "
                                              "{width, height}"}})
                                .dump());
                    return;
                }

                std::string cameraID;
                try {
                    cameraID = bufferAsJson["camera_id"].get<std::string>();
                } catch (const std::exception& e) {
                    res->writeStatus(HTTP_400_BAD_REQUEST)
                        ->endProblemJson(
                            (nlohmann::json {{"title",
                                              "required json "
                                              "field 'camera_id' of "
                                              "type string"}})
                                .dump());
                    return;
                }

                std::string cameraUri;
                try {
                    nldb::json camera = configurationDAO->GetCamera(cameraID);
                    cameraUri = camera["url"];
                } catch (const std::exception& e) {
                    res->writeStatus(HTTP_404_NOT_FOUND)
                        ->writeHeader("Cache-Control", "max-age=5")
                        ->endProblemJson(
                            (nlohmann::json {{"title",
                                              "camera with the specified id "
                                              "doesn't exists"}})
                                .dump());
                    return;
                }

                nlohmann::json initial = {{"camera_id", cameraID},
                                          {"duration", duration},
                                          {"date_unix", time(0)},
                                          {"state", "without_buffer"}};

                // instead of using random number I prefer to use the
                // db id, add a new object with the fps to get the an
                // id
                std::string id = videoBufferRepository->Add(initial);

                // async -> create the buffer task
                videoBufferTasksManager->AddTask(
                    Web::GenerateVideoBufferTask {.videoBufferTaskID = id,
                                                  .cameraID = cameraID,
                                                  .uri = cameraUri,
                                                  .duration = duration,
                                                  .delay = delay,
                                                  .resizeTo = resize});

                initial["id"] = id;

                res->writeStatus(HTTP_202_Accepted)->endJson(initial.dump());
            }
        });
    }

    template <bool SSL>
    void VideoBufferController<SSL>::GetAllBuffer(auto* res, auto* req) {
        std::string cameraID(req->getQuery("camera_id"));

        /**
         * state:
         *     - without_buffer: has id, camera_id, date_unix
         * and duration
         *     - with_buffer: has previous and fps
         *     - detected: has previous and contours, blobs
         */
        res->endJson((cameraID.empty()
                          ? videoBufferRepository->GetAll()
                          : videoBufferRepository->GetAll(cameraID))
                         .dump());
    }

    template <bool SSL>
    void VideoBufferController<SSL>::DeleteBuffer(auto* res, auto* req) {
        std::string id(req->getParameter(0));

        if (auto buffer = videoBufferRepository->GetInternal(id)) {
            if (buffer->contains("path")) {
                const std::string cameraFramesPath = buffer->at("path");
                remove(cameraFramesPath.c_str());
            }

            if (buffer->contains("diffFramesPath")) {
                const std::string diffFramesPath = buffer->at("diffFramesPath");
                remove(diffFramesPath.c_str());
            }

            videoBufferRepository->Remove(id);

            res->end();
        } else {
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=10")
                ->endProblemJson(
                    (nldb::json {{"title", "buffer not found"}}).dump());
            return;
        }
    }

    template <bool SSL>
    void VideoBufferController<SSL>::SteamBuffer(auto* res, auto* req) {
        std::string id(req->getParameter(0));
        std::string rangeHeader(req->getHeader("range"));

        // type = raw | diff
        std::string streamType(req->getQuery("type"));

        if (videoBufferRepository->Exists(id)) {
            std::optional<std::string> filename;

            if (streamType == "raw") {
                filename = videoBufferRepository->GetRawBufferPath(id);
            } else {
                filename = videoBufferRepository->GetDiffBufferPath(id);
            }

            if (!filename.has_value()) {
                res->writeStatus(HTTP_400_BAD_REQUEST)
                    ->endProblemJson(
                        (nldb::json {{"title",
                                      "buffer still doesn't have the "
                                      "frames requested"}})
                            .dump());
                return;
            }

            FileStreamer::GetInstance().streamFile(res, filename.value(),
                                                   rangeHeader);
        } else {
            res->writeStatus(HTTP_404_NOT_FOUND);
            res->end();
        }
    }
};  // namespace Web::Controller