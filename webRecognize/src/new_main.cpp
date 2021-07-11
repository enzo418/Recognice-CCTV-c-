/* This is a simple HTTP(S) web server much like Python's SimpleHTTPServer */

#include <uWebSockets/App.h>

#include "serverhelpers/AsyncFileReader.h"
#include "serverhelpers/AsyncFileStreamer.h"
#include "serverhelpers/Middleware.h"

#include "server_utils.hpp"

int main(int argc, char **argv) {
    /* ws->getUserData returns one of these */
    struct PerSocketData {
        /* Fill with user data */
    };

    int option;

    int port = 3000;
    char *root = "/home/cltx/projects/cpp/wxRecognize/webRecognize/build/web";

    AsyncFileStreamer asyncFileStreamer(root);

    uWS::App()

	.get("/", [&asyncFileStreamer](auto *res, auto *req) {
	    std::cout << "Index!" << std::endl;

        serveFile(res, req, true);
        
        asyncFileStreamer.streamFile(res, "/index.html");
        
        res->end();
    })

    .get("/api/configuration_files", [](auto* res, auto* req){
        res->writeHeader("Content-Type", "application/json");
        res->end(GetConfigurationsPathsJson({"../../recognize/build/", "./configurations/"}));
    })

	.get("/*", [&asyncFileStreamer](auto *res, auto *req) {
	    std::cout << "Any" << std::endl;
        serveFile(res, req);
        
        std::cout << "ASDASDASD" << std::endl;

        asyncFileStreamer.streamFile(res, req->getUrl());
        
        std::cout << "EEEEEEE" << std::endl;
        
        res->end();
    })

    .ws<PerSocketData>("recognize", {
        /* Settings */
        .compression = uWS::SHARED_COMPRESSOR,
        .maxPayloadLength = 16 * 1024 * 1024,
        .idleTimeout = 16,
        .maxBackpressure = 1 * 1024 * 1024,
        .closeOnBackpressureLimit = false,
        .resetIdleTimeoutOnSend = false,
        .sendPingsAutomatically = true,
        /* Handlers */
        .upgrade = nullptr,
        .open = [](auto */*ws*/) {
            /* Open event here, you may access ws->getUserData() which points to a PerSocketData struct */
        },
        .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {
            // echo message
            ws->send(message, opCode, true);
        },
        .drain = [](auto */*ws*/) {
            /* Check ws->getBufferedAmount() here */
        },
        .ping = [](auto */*ws*/, std::string_view) {
            /* Not implemented yet */
        },
        .pong = [](auto */*ws*/, std::string_view) {
            /* Not implemented yet */
        },
        .close = [](auto */*ws*/, int /*code*/, std::string_view /*message*/) {
            /* You may access ws->getUserData() here */
        }
	})
	
    .listen(port, [port, root](auto *token) {
            if (token) {
                std::cout << "Serving " << root << " over HTTP a " << port << std::endl;
            }
    })
        
    .run();

    std::cout << "Failed to listen to port " << port << std::endl;
}
