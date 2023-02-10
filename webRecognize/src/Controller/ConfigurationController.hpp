#include "DAL/IConfigurationDAO.hpp"
#include "Utils/JsonUtils.hpp"
#include "server_utils.hpp"
#include "uWebSockets/App.h"

namespace Web::Controller {
    template <bool SSL>
    class ConfigurationController {
       public:
        ConfigurationController(uWS::App* app,
                                Web::DAL::IConfigurationDAO* configurationDAO);

        void GetAllConfiguration(auto* res, auto* req);
        void CreateOrCloneConfiguration(auto* res, auto* req);
        void CreateOrCloneCameraConfiguration(auto* res, auto* req);
        void DeleteConfiguration(auto* res, auto* req);
        void DeleteCameraConfiguration(auto* res, auto* req);
        void GetConfigurationField(auto* res, auto* req);
        void UpdateConfigurationField(auto* res, auto* req);

       private:
        Web::DAL::IConfigurationDAO* configurationDAO;
    };

    template <bool SSL>
    ConfigurationController<SSL>::ConfigurationController(
        uWS::App* app, Web::DAL::IConfigurationDAO* pConfigurationDAO)
        : configurationDAO(pConfigurationDAO) {
        app->get("/api/configuration/", [this](auto* res, auto* req) {
            this->GetAllConfiguration(res, req);
        });

        // create a new configuration
        app->post("/api/configuration/", [this](auto* res, auto* req) {
            this->CreateOrCloneConfiguration(res, req);
        });

        // create a new camera
        app->post("/api/configuration/:id/camera/",
                  [this](auto* res, auto* req) {
                      this->CreateOrCloneCameraConfiguration(res, req);
                  });

        // delete a configuration
        app->del("/api/configuration/:id", [this](auto* res, auto* req) {
            this->DeleteConfiguration(res, req);
        });

        // delete a camera
        app->del("/api/configuration/:id/camera/:cam_id",
                 [this](auto* res, auto* req) {
                     this->DeleteCameraConfiguration(res, req);
                 });

        // get a configuration field
        app->get("/api/configuration/:id", [this](auto* res, auto* req) {
            this->GetConfigurationField(res, req);
        });

        // update a configuration field
        app->post("/api/configuration/:id", [this](auto* res, auto* req) {
            this->UpdateConfigurationField(res, req);
        });
    }

    template <bool SSL>
    void ConfigurationController<SSL>::GetAllConfiguration(auto* res,
                                                           auto* req) {
        // Get all the configurations id and name available
        res->endJson(configurationDAO->GetAllNamesAndId().dump());
    }

    template <bool SSL>
    void ConfigurationController<SSL>::CreateOrCloneConfiguration(auto* res,
                                                                  auto* req) {
        res->onAborted([]() {});

        std::string buffer;

        // get it before we attach the reader
        auto ct = std::string(req->getHeader("content-type"));

        res->onData([this, req, res, ct, buffer = std::move(buffer)](
                        std::string_view data, bool last) mutable {
            buffer.append(data.data(), data.length());

            if (last) {
                //   std::cout << "buffer: " << buffer << std::endl;

                if (ct != "application/json") {
                    res->writeStatus(HTTP_400_BAD_REQUEST)
                        ->end("Expected a json body");
                    return;
                }

                Observer::Configuration configuration;
                nldb::json bufferAsJson;

                // default response is an error
                nldb::json response = {
                    {"status", 400},
                    {"title", "invalid configuration"},
                    {"detail",
                     "we couldn't parse the configuration, check "
                     "that it has all the fields needed"}};

                try {
                    bufferAsJson = nlohmann::json::parse(buffer);

                    if (!bufferAsJson.empty()) {
                        // should clone
                        if (bufferAsJson.contains("clone_id")) {
                            // move this code into a clone configuration
                            // method that takes a json

                            if (!bufferAsJson["clone_id"].is_string()) {
                                res->writeStatus(HTTP_400_BAD_REQUEST)
                                    ->endProblemJson(
                                        (nlohmann::json {{"title",
                                                          "clone_id must be of "
                                                          "type string"}})
                                            .dump());
                                return;
                            }

                            nldb::json config;
                            try {
                                config = configurationDAO->GetConfiguration(
                                    bufferAsJson["clone_id"]);
                            } catch (const std::exception& e) {
                                res->writeStatus(HTTP_404_NOT_FOUND)
                                    ->writeHeader("Cache-Control", "max-age=5")
                                    ->endProblemJson(
                                        (nlohmann::json {{"title",
                                                          "configuration not "
                                                          "found"}})
                                            .dump());
                                return;
                            }

                            // erase the id so it generates a new
                            // one, also change its name to name +
                            // "copy"

                            config["name"] =
                                config["name"].get<std::string>() + " copy";

                            config.erase("_id");
                            config.erase("id");

                            bufferAsJson = config;
                        }

                        configuration = bufferAsJson;
                    } else {
                        bufferAsJson = configuration;
                    }

                    std::string id =
                        configurationDAO->InsertConfiguration(bufferAsJson);

                    // set success response
                    response = {{"id", id}};
                    res->writeHeader("Content-Type", "application/json");
                } catch (const std::exception& e) {
                    OBSERVER_WARN("Configuration wasn't added: {}", e.what());

                    res->writeStatus(HTTP_400_BAD_REQUEST);
                    res->writeHeader("Content-Type",
                                     "application/problem+json");
                }

                res->end(response.dump());
            }
        });
    }

    template <bool SSL>
    void ConfigurationController<SSL>::CreateOrCloneCameraConfiguration(
        auto* res, auto* req) {
        res->onAborted([]() {});

        std::string buffer;

        auto ct = std::string(req->getHeader("content-type"));

        res->onData([this, res, req, ct, buffer = std::move(buffer)](
                        std::string_view data, bool last) mutable {
            buffer.append(data.data(), data.length());

            if (last) {
                if (ct != "application/json") {
                    res->writeStatus(HTTP_400_BAD_REQUEST)
                        ->end("Expected a json body");
                    return;
                }

                nlohmann::json parsed;

                try {
                    parsed = nlohmann::json::parse(buffer);
                } catch (const std::exception& e) {
                    res->writeStatus(HTTP_400_BAD_REQUEST)
                        ->endProblemJson(
                            (nlohmann::json {
                                 {"title", "body isn't a valid json"}})
                                .dump());
                    return;
                }

                auto configurationID = std::string(req->getParameter(0));

                // creat a instance with the default value
                Observer::CameraConfiguration cam;
                nldb::json jsonCamera;

                if (parsed.contains("clone_id")) {
                    // clone the camera
                    if (!parsed["clone_id"].is_string()) {
                        res->writeStatus(HTTP_400_BAD_REQUEST)
                            ->endProblemJson(
                                (nlohmann::json {{"title",
                                                  "clone_id must be of "
                                                  "type string"}})
                                    .dump());
                        return;
                    }

                    try {
                        jsonCamera =
                            configurationDAO->GetCamera(parsed["clone_id"]);

                        // erase id
                        jsonCamera.erase("_id");
                        jsonCamera.erase("id");

                        // change name to copy
                        jsonCamera["name"] =
                            jsonCamera["name"].get<std::string>() + " copy";

                    } catch (const std::exception& e) {
                        res->writeStatus(HTTP_400_BAD_REQUEST)
                            ->endProblemJson(
                                (nlohmann::json {
                                     {"title", "camera to clone not found"}})
                                    .dump());
                        return;
                    }
                } else {
                    cam.name = "new camera";

                    // parse the default camera to json
                    jsonCamera = cam;
                }

                try {
                    std::string newCameraID =
                        configurationDAO->AddCameraToConfiguration(
                            configurationID, jsonCamera);

                    res->endJson((nlohmann::json {{"id", newCameraID}}).dump());
                } catch (const std::exception& e) {
                    OBSERVER_WARN("Could not add the new camera: {}", e.what());

                    res->writeStatus(HTTP_400_BAD_REQUEST)
                        ->endProblemJson(
                            (nlohmann::json {
                                 {"title", "couldn't add the new camera"}})
                                .dump());
                }
            }
        });
    }

    template <bool SSL>
    void ConfigurationController<SSL>::DeleteConfiguration(auto* res,
                                                           auto* req) {
        auto configID = std::string(req->getParameter(0));
        configurationDAO->DeleteConfiguration(configID);

        res->writeStatus(HTTP_204_NO_CONTENT)->end();
    }

    template <bool SSL>
    void ConfigurationController<SSL>::DeleteCameraConfiguration(auto* res,
                                                                 auto* req) {
        auto configID = std::string(req->getParameter(0));
        auto cameraID = std::string(req->getParameter(1));

        try {
            configurationDAO->DeleteCameraFromConfiguration(configID, cameraID);
        } catch (const std::exception& e) {
            res->writeStatus(HTTP_400_BAD_REQUEST)
                ->endProblemJson(
                    (nlohmann::json {{"title", "could not delete this camera"}})
                        .dump());

            return;
        }

        res->writeStatus(HTTP_204_NO_CONTENT)->end();
    }

    template <bool SSL>
    void ConfigurationController<SSL>::GetConfigurationField(auto* res,
                                                             auto* req) {
        std::string url(req->getUrl());
        auto id = req->getParameter(0);
        std::string fieldPath(req->getQuery("field"));

        nldb::json obj;
        try {
            obj = configurationDAO->GetConfiguration(std::string(id));
        } catch (const std::exception& e) {
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->end("Configuration not found");
            return;
        }

        // Get the camera from the database if requested
        auto camerasPos = fieldPath.find("cameras/");
        if (fieldPath.find("cameras/") != fieldPath.npos) {
            fieldPath = fieldPath.substr(camerasPos + 8 /*cameras/*/);
            auto nextSlash = fieldPath.find("/");
            if (nextSlash != fieldPath.npos) {
                // get the camera
                auto cameraID = fieldPath.substr(0, nextSlash);
                try {
                    obj = configurationDAO->GetCamera(cameraID);
                } catch (const std::exception& e) {
                    res->writeStatus(HTTP_404_NOT_FOUND)
                        ->writeHeader("Cache-Control", "max-age=5")
                        ->end("Camera not found");
                    return;
                }

                // remove the id from the path
                fieldPath = fieldPath.substr(nextSlash);
            }
        }

        nldb::json value;
        try {
            value = Observer::ConfigurationParser::GetConfigurationFieldValue(
                obj, fieldPath);
        } catch (const std::exception& e) {
            std::cout << "Couldn't get configuration field: " << e.what()
                      << std::endl;
            res->writeStatus(HTTP_400_BAD_REQUEST)->end();
            return;
        }

        if (value.is_null()) {
            res->writeStatus(HTTP_404_NOT_FOUND)
                ->writeHeader("Cache-Control", "max-age=5")
                ->end();
            return;
        }

        res->endJson(value.dump());
    }

    template <bool SSL>
    void ConfigurationController<SSL>::UpdateConfigurationField(auto* res,
                                                                auto* req) {
        res->onAborted([]() {});

        std::string buffer;

        // get it before we attach the reader
        auto ct = std::string(req->getHeader("content-type"));

        res->onData([this, res, req, ct, buffer = std::move(buffer)](
                        std::string_view data, bool last) mutable {
            buffer.append(data.data(), data.length());

            if (last) {
                if (ct != "application/json") {
                    res->writeStatus(HTTP_400_BAD_REQUEST)
                        ->end("Expected a json body");
                    return;
                }

                nlohmann::json parsed;
                try {
                    parsed = nlohmann::json::parse(buffer);
                } catch (const std::exception& e) {
                    res->writeStatus(HTTP_400_BAD_REQUEST)
                        ->end("Body is not a valid json");
                    return;
                }

                if (parsed.is_null() || !parsed.contains("field") ||
                    !parsed.contains("value") || !parsed["field"].is_string()) {
                    res->writeStatus(HTTP_400_BAD_REQUEST)
                        ->end(
                            "Expected json members: field and "
                            "value");
                    return;
                }

                std::string lastPathKey;
                std::string fieldPath = parsed["field"].get<std::string>();

                auto camerasPos = fieldPath.find("cameras/");
                if (fieldPath.find("cameras/") != fieldPath.npos) {
                    // UPDATE A CAMERA
                    fieldPath = fieldPath.substr(camerasPos + 8 /*cameras/*/);
                    auto nextSlash = fieldPath.find("/");
                    if (nextSlash != fieldPath.npos) {
                        // get the camera
                        auto cameraID = fieldPath.substr(0, nextSlash);

                        // remove the id from the path
                        fieldPath = fieldPath.substr(nextSlash);

                        // TODO: ensure that cameraID belongs to
                        // parameter.id

                        nldb::json updated = Web::GenerateJsonFromPath(
                            fieldPath, parsed["value"], &lastPathKey);

                        try {
                            configurationDAO->UpdateCamera(cameraID, updated);
                        } catch (const nldb::WrongPropertyType& e) {
                            nldb::json problem = {
                                {"title", "Wrong property type"},
                                {"invalidParams",
                                 {{lastPathKey, {{"reason", e.what()}}}}}};

                            res->writeStatus(HTTP_400_BAD_REQUEST)
                                ->writeHeader("Content-Type",
                                              "application/problem+json")
                                ->end(problem.dump());
                        } catch (...) {
                            res->writeStatus(HTTP_500_INTERNAL_SERVER_ERROR)
                                ->end();
                        }
                    }
                } else {
                    // UPDATE A CONFIGURATION
                    std::string id(req->getParameter(0));

                    nldb::json updated = Web::GenerateJsonFromPath(
                        fieldPath, parsed["value"], &lastPathKey);

                    try {
                        configurationDAO->UpdateConfiguration(id, updated);
                    } catch (const nldb::WrongPropertyType& e) {
                        nldb::json problem = {
                            {"title", "Wrong property type"},
                            {"invalidParams",
                             {{lastPathKey, {{"reason", e.what()}}}}}};

                        res->writeStatus(HTTP_400_BAD_REQUEST)
                            ->writeHeader("Content-Type",
                                          "application/problem+json")
                            ->end(problem.dump());
                    } catch (...) {
                        res->writeStatus(HTTP_500_INTERNAL_SERVER_ERROR)->end();
                    }
                }

                res->end("field updated");
            }
        });
    }
}  // namespace Web::Controller