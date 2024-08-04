#pragma once

#include <future>

#include "mdns.hpp"
#include "nlohmann/json.hpp"
#include "server_utils.hpp"
#include "uWebSockets/App.h"

namespace Web::Controller {
    template <bool SSL>
    class AIServerController {
       public:
        AIServerController(uWS::App* app);

        void SearchServer(auto* res, auto* req);

       private:
        mdns::MDNSClient mdnsClient;
        std::vector<std::future<void>> asyncTasks;

        uWS::Loop* loop;
    };

    template <bool SSL>
    AIServerController<SSL>::AIServerController(uWS::App* app)
        : mdnsClient("_darknet._tcp.local.") {
        app->get("/api/ai/find-any-server", [this](auto* res, auto* req) {
            this->SearchServer(res, req);
        });
        loop = uWS::Loop::get();
    }

    template <bool SSL>
    void AIServerController<SSL>::SearchServer(auto* res, auto* req) {
        res->onAborted([]() {});

        this->asyncTasks.push_back(
            std::async(std::launch::async, [this, res]() {
                std::future<std::optional<mdns::ServiceFound>> result =
                    mdnsClient.findService(/*timeout*/ 1, /*wait_for_txt*/ true,
                                           /*wait_for_bothIP46*/ false);

                result.wait();
                if (result.valid()) {
                    auto service = result.get();
                    if (service.has_value() && service->ipv4_addr.has_value() &&
                        service->port.has_value()) {
                        nlohmann::json response = {
                            {"hostname", service->host_name},
                            {"ipv4_addr", service->ipv4_addr.value()},
                            {"port", service->port.value()}};

                        loop->defer([res, response]() {
                            res->endJson(response.dump());
                        });

                        return;
                    }
                }

                loop->defer([res]() {
                    nlohmann::json response = {{"title", "No server online."}};
                    res->writeStatus(HTTP_404_NOT_FOUND)
                        ->writeHeader("Cache-Control", "max-age=5")
                        ->endProblemJson(nlohmann::json({}).dump());
                });
            }));
    }
}  // namespace Web::Controller