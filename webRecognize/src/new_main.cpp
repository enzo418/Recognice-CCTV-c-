#include <opencv2/core.hpp>

#include "../../recognize/src/recognize.hpp"
#include "../../recognize/src/configuration_file.hpp"
#include "../../recognize/src/notification.hpp"
#include "../../recognize/src/utils.hpp"

#include <uWebSockets/App.h>
#include <uWebSockets/HttpContextData.h>
#include <uWebSockets/Multipart.h>

#include "serverhelpers/AsyncFileReader.h"
#include "serverhelpers/AsyncFileStreamer.h"
#include "serverhelpers/Middleware.h"

#include "server_utils.hpp"

// Selects a Region of interes from a camera fram
#include "AreaSelector.hpp"

#include "base64.hpp"

#include <filesystem>
#include <cstring>

namespace fs = std::filesystem;

int main(int argc, char **argv) {
    // TODO: Change App to handle post data and call the handler
    // when it reads all

    /* ws->getUserData returns one of these */
    struct PerSocketData {
        /* Fill with user data */
    };

    int option;

    int port = 3001;
    const std::string serverRootFolder = "/home/cltx/projects/cpp/wxRecognize/webRecognize/build/web";

    AsyncFileStreamer asyncFileStreamer(serverRootFolder);
    
    // Recognize program
    Recognize* recognize;

    // TODO: Move this var to Recognize. Him should know if is active or not!
    bool recognize_running = false;

    // This saves the content of each chunk of the post body
    std::string postBody;

    // Configuration that the recognize program uses when we start it
    Configurations current_configurations;
    
    // where all the videos/images are stored this gets a value each time
    // we start the recognizer with a configuration file
    std::string mediaPath = "";

    // cache camera frames requested to improve the user experience
    // TODO: add timeout
    std::map<std::string, std::string> cachedImages;

    // Stores all the notifications from all time (read at start and written to disk in intervals)
    Json::Value persintent_notifications;

    uWS::App()

	.get("/", [&asyncFileStreamer](auto *res, auto *req) {
	    std::cout << "Index!" << std::endl;

        setFileContentType(res, req->getUrl(), true);
        
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

                ConfigurationFile::SaveConfigurations(cfgs, path.c_str());
            }

            // send the file
            res->end(GetJsonString("configuration_file", Json::Value(stringConfigs).toStyledString()));
        } else {
            res->end(GetAlertMessage(AlertStatus::ERROR, "File could not be read, there is an invalid field", error));
        }
    })

    /** writes a configuration into a file
     *  Send the filename as a query, file_name=test.ini
     *  Send the file content as the post body
    */
    .post("/api/configuration_file", [&postBody](auto* res, uWS::HttpRequest* req) {
        std::string type(req->getHeader("content-type"));
        std::cout << "content-type: " << type << std::endl;
        std::cout << "content-length: "<< req->getHeader("content-length") << std::endl;

        std::string length(req->getHeader("content-length"));
        std::string file(req->getQuery("file_name"));

        postBody.resize(std::stoi(length));

        postBody.clear();
        
        req->setYield(false);
        res->onAborted([]() {
            std::cout << "Aborted" << std::endl;
        });
        
        res->onData([&postBody, fileName = std::move(file), contentType = std::move(type), response = std::move(res), request = std::move(req)](std::string_view chunk, bool isLast) {
            postBody.append(chunk);
            std::cout   << "Chunk length: " << chunk.length() 
                        << " is last? " << (isLast ? "true" : "false") << std::endl<< std::endl;
            // std::cout << "Body: " << postBody << std::endl;

            if (isLast) {

                // response is a json
                response->writeHeader("Content-Type", "application/json");
                
                if (HTTP_MULTIPART == contentType || HTTP_FORM_URLENCODED == contentType) {
                    std::cout << std::endl;
                    
                    std::cout 
                        << "File=" << fileName
                        << "\nConfiguration size:\n" << postBody.length() << std::endl;
                    
                    std::string error;
                    std::istringstream iss(postBody);
                    Configurations configurations = ConfigurationFile::ReadConfigurationBuffer(iss, error);

                    if (error.length() == 0) {
                        std::cout << "There is " << configurations.camerasConfigs.size() << " cameras in the string\n";
                        ConfigurationFile::SaveConfigurations(configurations, fileName);

                        response->end(GetAlertMessage(AlertStatus::OK, "File saved correctly"));
                    } else {
                        response->end(GetAlertMessage(AlertStatus::ERROR, "File could not be saved, there is an invalid field", error));
                    }
                } else {
                    // if it's json ¿?
                    // Throw error   
                    response->end();
                }
            }
        });
    })

    /** 
     * Start the recognizer
     * Send as a query the file path to use, e.g. file_name=test.ini
    **/
    .get("/api/start_recognizer", [&current_configurations, &mediaPath, &serverRootFolder, &recognize, &recognize_running](auto *res, auto *req) {
        std::string file(req->getQuery("file_name"));
        std::cout << "Starting recognize with file: " << file << " empty? " << file.empty() << std::endl;
        bool success = false;
        
        // response is a json
        res->writeHeader("Content-Type", "application/json");

        if (!file.empty()) {
            if (!recognize_running) {
                std::string error;
                Configurations configurations = ConfigurationFile::ReadConfigurations(file, error);
                if (error.length() == 0) {
                    current_configurations = configurations;

                    std::cout << "Config cameras size: " << configurations.camerasConfigs.size() << std::endl;

                    fs::create_directories(configurations.programConfig.imagesFolder);

                    mediaPath = configurations.programConfig.imagesFolder.substr(
                                        serverRootFolder.size(), 
                                        configurations.programConfig.imagesFolder.size()
                                    );

                    if (recognize->Start(std::move(configurations), 
                                        configurations.programConfig.showPreview, 
                                        configurations.programConfig.telegramConfig.useTelegramBot)) {							
                        res->end(GetAlertMessage(AlertStatus::OK, "Recognizer started"));
                        success = true;
                    } else {
                        res->end(GetAlertMessage(AlertStatus::ERROR, "Could not start the recognizer, check that the configuration file has active cameras.", error));
                    }
                } else {
                    res->end(GetAlertMessage(AlertStatus::ERROR, "File could not be read, there is an invalid field", error));
                }
            } else {
                res->end(GetAlertMessage(AlertStatus::ERROR, "Recognizer could not be started because it is already running"));
            }
        } else {
            res->end(GetAlertMessage(AlertStatus::ERROR, "No file in request."));
        }

        if (success) {
            recognize_running = !recognize_running;
            // TODO: Notifiy all users that join the websocket
            // sendEveryone(GetRecognizeStateJson(recognize_running));
        }
    })

    .get("/api/stop_recognizer", [&recognize](auto *res, auto *req) {
	    recognize->CloseAndJoin();

        // response is a json
        res->writeHeader("Content-Type", "application/json");

        res->end(GetAlertMessage(AlertStatus::OK, "Recognizer stopped"));
        std::cout << "Closed recognize" << std::endl;
    })

    .get("/api/camera_frame", [&cachedImages](auto *res, auto *req) {
	    const unsigned int index = std::stoi(static_cast<std::string>(req->getQuery("index")));
        const int rotation = std::stoi(static_cast<std::string>(req->getQuery("rotation")));
        const std::string_view url = req->getQuery("url");
        const std::string_view roi_s = req->getQuery("roi");

        // response is a json
        res->writeHeader("Content-Type", "application/json");

        if (!url.empty()) {
            std::string cacheKey;
            cacheKey.append(url).append(std::to_string(rotation)).append(roi_s);
            std::string encoded;
            bool error = false;
            if (cachedImages.find(cacheKey) == cachedImages.end()) {							
                cv::Mat img;
                if (AreaSelector::GetFrame(url.data(), img)) {		
                    AreaSelector::ResizeRotateFrame(img, rotation);

                    if (!roi_s.empty()) {
                        const std::vector<int> numbers = Utils::GetNumbersString(static_cast<std::string>(roi_s));
                        if (numbers.size() == 4) {
                            cv::Rect roi(cv::Point(numbers[0], numbers[1]), cv::Size(numbers[2], numbers[3]));
                            img = img(roi);
                        }
                    }

                    std::vector<uchar> buf;
                    cv::imencode(".jpg", img, buf);
                    auto *enc_msg = reinterpret_cast<unsigned char*>(buf.data());
                    encoded = base64_encode(enc_msg, buf.size());
                    cachedImages.insert({cacheKey, encoded});
                } else {
                    res->end(GetAlertMessage(AlertStatus::ERROR, "Could not open a connection to the camera"));
                    error = true;
                }
            } else {
                encoded = cachedImages[cacheKey];
            }

            if (!error)
                res->end(GetJsonString("camera_frame", GetJsonString({{"camera", std::to_string(index)},{"frame", encoded}})));
        } else {
            res->end(GetAlertMessage(AlertStatus::ERROR, "The camera url is empty"));
        }
    })

    .get("/api/new_camera", [&](auto *res, auto *req) {
        CameraConfiguration cfg;
        Json::Value root;

        root["new_camera_config"]["configuration"] = Json::Value(ConfigurationFile::GetConfigurationString(cfg));

        // response is a json
        res -> writeHeader("Content-Type", "application/json")
            -> end(root.toStyledString());
    })

    .post("/api/copy_file", [&](auto *res, auto *req) {
        const auto file = req->getQuery("file");
        const auto copy_path = req->getQuery("copy_path");

        // response is a json
        res->writeHeader("Content-Type", "application/json");

        std::cout 	<< "File requested=" << file
                    << std::endl
                    << "Copied to: " << copy_path
                    << std::endl;
        
        // create directory if missing
        std::filesystem::path path { copy_path };
        std::filesystem::create_directories(path.parent_path());
        
        // copy file
        fs::copy_file(file, copy_path);

        // read file
        std::string error;
        Configurations cfgs = ConfigurationFile::ReadConfigurations(copy_path.data(), error);
        if (error.length() == 0) {
            std::string stringCfgs = ConfigurationFile::ConfigurationsToString(cfgs);

            // send configuration
            res->end(GetJsonString("configurations", Json::Value(stringCfgs).toStyledString()));
        } else {
            res->end(GetAlertMessage(AlertStatus::ERROR, "The copied file was invalid and now you have 2 invalid files", error));
        }
    })

    .get("/api/notifications", [&persintent_notifications](auto *res, auto *req) {
        // response is a json
        res->writeHeader("Content-Type", "application/json");
	    res->end(GetJsonString("notifications", persintent_notifications.toStyledString()));
    })

	.get("/*.*", [&asyncFileStreamer](auto *res, auto *req) {
	    std::cout << "2. Any file" << std::endl;

        if (!hasExtension(req->getUrl())) {
            req->setYield(true);
        } else if (asyncFileStreamer.streamFile(res, req->getUrl())){
            std::cout << "Succesfull sended file" << std::endl;
        
            res->end();
        } else {
            res->end();
        }
    })

    .get("/*", [](auto *res, auto *req) {
	    std::cout << "3. Web" << std::endl;
        res->writeStatus(HTTP_301_MOVED_PERMANENTLY);
        res->writeHeader("Location", "/");
        res->writeHeader("Content-Type", "text/html");
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
	
    .listen(port, [port, serverRootFolder](auto *token) {
            if (token) {
                std::cout << "Serving " << serverRootFolder << " over HTTP a " << port << std::endl;
            }
    })
        
    .run();

    std::cout << "Failed to listen to port " << port << std::endl;
}
