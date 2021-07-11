/* This is a simple HTTP(S) web server much like Python's SimpleHTTPServer */

#include <uWebSockets/App.h>
#include <uWebSockets/HttpContextData.h>
#include <uWebSockets/Multipart.h>

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

    int port = 3001;
    const char *root = "/home/cltx/projects/cpp/wxRecognize/webRecognize/build/web";

    AsyncFileStreamer asyncFileStreamer(root);

    uWS::App()

	.get("/", [&asyncFileStreamer](auto *res, auto *req) {
	    std::cout << "Index!" << std::endl;

        serveFile(res, req, true);
        
        asyncFileStreamer.streamFile(res, "/index.html");
        
        res->end();
    })

    // To use parameters put the parameter in the url like /path/:file
    // then use req->getParameter(0) to get the parameter

    .get("/api/configuration_files", [](auto* res, uWS::HttpRequest* req){
        res->writeHeader("Content-Type", "application/json");
        res->end(GetConfigurationsPathsJson({"../../recognize/build/", "./configurations/"}));
        
        // not handle the route, causing the router to continue looking 
        // for a matching route handler, or fail
        // req->setYield(true);
    })

    // answer with the request configuration file
    .get("/api/configuration_file", [](auto* res, uWS::HttpRequest* req) {
        /**
         * Since we are working with files, this should response with a 
         * chunked response sending the files but for now it will just
         * send it as a string in Json format.
        */

        std::string file(req->getQuery("file"));

        // if the file doesn't exist right now
        const bool isNew = req->getQuery("is_new") == "true" ? true : false;

        std::cout << "Query file: " << file << std::endl;
        
        std::string error;

        // response is a json
        res->writeHeader("Content-Type", "application/json");

        // read file
        Configurations cfgs = ConfigurationFile::ReadConfigurations(file, error);

        if (error.length() == 0) {
            std::string stringConfigs = ConfigurationFile::ConfigurationsToString(cfgs);

            if (isNew) {
                std::filesystem::path path { file };
                
                std::cout << "\tFile is new. Full path: " << path << std::endl;

                std::filesystem::create_directories(path.parent_path());

                ConfigurationFile::SaveConfigurations(cfgs, path.string());
            }

            // send the file
            res->end(GetJsonString("configuration_file", Json::Value(stringConfigs).toStyledString()));
        } else {
            res->end(GetAlertMessage(AlertStatus::ERROR, "File could not be read, there is an invalid field", error));
        }
    })

    // writes a configuration into a file
    .post("/api/configuration_file", [](auto* res, uWS::HttpRequest* req) {
        std::cout << "content-type: " << req->getHeader("content-type") << std::endl;
        std::cout << "content-length: "<< req->getHeader("content-length") << std::endl;
        if (multipart == req->getHeader("content-type")) {
            std::string length(req->getHeader("content-length"));
            std::string reqBody(multipart);

            reqBody.append(";");

            reqBody.resize(strlen(multipart) + 1 + std::stoi(length));

            std::cout << std::endl;
            
            std::cout << "¿?: "<< req->getHeader(req->getUrl()) << std::endl;
            std::cout << "Query?: "<< req->getQuery() << std::endl;

            res->onData([&](std::string_view chunk, bool isLast) {
                reqBody.append(chunk);
                std::cout << "Chunk: " << chunk << " is last? " << (isLast ? "true" : "false") << std::endl;
                if (isLast) {
                    uWS::MultipartParser parser(std::string_view(reqBody.c_str(), reqBody.length()));
                    std::cout << "Is valid? " << parser.isValid() << std::endl;
                    for (size_t i = 0; i < 10; i++)
                    {
                        std::cout << i << ": " << parser.getNextPart() << std::endl;
                    }
                    
                }
            });
            
            std::string file(req->getParameter(0));
            // const std::string cfgString = root["configurations"].asString();
            std::cout 
                << "File=" << file
                /*<< "Configuration size:\n" << cfgString.length()*/ << std::endl;
            
            // std::string error;
            // std::istringstream iss(cfgString);
            // Configurations configurations = ConfigurationFile::ReadConfigurationBuffer(iss, error);

            // if (error.length() == 0) {
            //     std::cout << "There is " << configurations.camerasConfigs.size() << " cameras in the string\n";

            //     ConfigurationFile::SaveConfigurations(configurations, file);

            //     con->send(GetAlertMessage(AlertStatus::OK, "File saved correctly"));
            // } else {
            //     con->send(GetAlertMessage(AlertStatus::ERROR, "File could not be saved, there is an invalid field", id, error));
            // }
            res->end();
        }
    })

	.get("/*", [&asyncFileStreamer](auto *res, auto *req) {
	    std::cout << "2. Any" << std::endl;
        serveFile(res, req);

        if (asyncFileStreamer.streamFile(res, req->getUrl())){
            res->end();
        }
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
