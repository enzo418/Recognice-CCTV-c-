#pragma once

#include "Serialization/JsonSerialization.hpp"
#include "Server/ServerConfigurationProvider.hpp"
#include "Utils/JsonUtils.hpp"
#include "server_utils.hpp"
#include "uWebSockets/App.h"

namespace Web::Controller {
    template <bool SSL>
    class ServerConfigurationController {
       public:
        ServerConfigurationController(uWS::App* app);

        void GetField(auto* res, auto* req);
        void UpdateField(auto* res, auto* req);
    };

    template <bool SSL>
    ServerConfigurationController<SSL>::ServerConfigurationController(
        uWS::App* app) {
        app->get("/api/server_configuration",
                 [this](auto* res, auto* req) { this->GetField(res, req); });

        app->post("/api/server_configuration", [this](auto* res, auto* req) {
            this->UpdateField(res, req);
        });
    }

    template <bool SSL>
    void ServerConfigurationController<SSL>::GetField(auto* res, auto* req) {
        std::string fieldPath(req->getQuery("field"));

        nlohmann::json current = ServerConfigurationProvider::Get();

        nlohmann::json value;
        try {
            value = Observer::ConfigurationParser::GetConfigurationFieldValue(
                current, fieldPath);
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
    void ServerConfigurationController<SSL>::UpdateField(auto* res, auto* req) {
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

                nlohmann::json fields = Web::GenerateJsonFromPath(
                    fieldPath, parsed["value"], &lastPathKey);

                try {
                    ServerConfigurationProvider::Update(fields);
                } catch (const nlohmann::json::exception& e) {
                    nlohmann::json problem = {
                        {"title", "unable to update the field"},
                        {"invalidParams",
                         {{lastPathKey, {{"reason", e.what()}}}}}};

                    res->writeStatus(HTTP_400_BAD_REQUEST)
                        ->writeHeader("Content-Type",
                                      "application/problem+json")
                        ->end(problem.dump());
                } catch (...) {
                    res->writeStatus(HTTP_500_INTERNAL_SERVER_ERROR)->end();
                }

                res->end("field updated");
            }
        });
    }

}  // namespace Web::Controller