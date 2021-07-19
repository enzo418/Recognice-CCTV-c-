#include <opencv2/core.hpp>

#include "../../recognize/src/recognize.hpp"
#include "../../recognize/src/configuration_file.hpp"
#include "../../recognize/src/notification.hpp"
#include "../../recognize/src/utils.hpp"

#include "../uWebSockets/src/App.h"
#include "../uWebSockets/src/HttpContextData.h"
#include "../uWebSockets/src/Multipart.h"

#include "stream_content/FileReader.h"
#include "stream_content/FileStreamer.h"
#include "stream_content/FileExtension.h"

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

    FileStreamer fileStreamer(serverRootFolder);
    
    // Recognize program
    Recognize recognize;

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
    const std::string PERSISTENT_NOTIFICATIONS_FILE = "./notifications.json";

    // stores the last 20 notification sended to each client
    // so when the web is restated, the client has some 
    // notifications from the current run
    std::vector<std::string> notificationsSended(20);
    int currentNotificationIndex = 0;

	// read the file with all the notifications until now
	ReadNotificationsFile(PERSISTENT_NOTIFICATIONS_FILE, std::ref(persintent_notifications));

	// start the thread to write the notifications to disk
	std::thread disk_notifications([&PERSISTENT_NOTIFICATIONS_FILE, &persintent_notifications] {
		Json::FastWriter writer;
		for(;;) {
			std::this_thread::sleep_for(std::chrono::seconds(20));
			WriteNotificationsFile(PERSISTENT_NOTIFICATIONS_FILE, std::ref(persintent_notifications), std::ref(writer));
		}
	});
	disk_notifications.detach();

    // stores all the websocket clients
    // TODO: Remove clients that are no longer connected
    std::vector<uWS::WebSocket<false, true, PerSocketData>*> clients;

    // send a message to all the web socket clients
    auto sendToEveryone = [&clients](const std::string& message) {
        for (auto & client : clients) {
            if (client->isSubscribed("/recognize")) {
                client->send(message, uWS::OpCode::TEXT, true);
            }
        }
    };

	// start thread that send the notifications to the web
	std::thread tick([&] {
		for(;;) {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			if (recognize_running) {
			    std::tuple<Notification::Type, std::string, std::string, std::string> media;
				while (recognize.notificationWithMedia->try_dequeue(media)) {	
					std::string type_string;
					std::string query = "";
					Notification::Type type; std::string content; std::string group_id; std::string datetime;

					std::tie(type, content, group_id, datetime) = media;

					if (type == Notification::IMAGE || type == Notification::GIF || type == Notification::VIDEO) {
						query = type == Notification::VIDEO ? "video" : "image";
						
						std::size_t found = content.find_last_of("/\\");
						content = "/media/" + content.substr(found+1);
					} else if (type == Notification::TEXT) {
						query = "text";
					} else if (type == Notification::SOUND) {
						query = "sound";
					}

					type_string = query;
					const std::string body = fmt::format("{{\"type\":\"{0}\", \"content\":\"{1}\", \"group\":\"{2}\", \"datetime\":\"{3}\"}}", query, content, group_id, datetime);
					query = fmt::format("{{\"new_notification\": {}}}", body);

					notificationsSended[currentNotificationIndex] = body;
					
					AppendNotification(persintent_notifications, type_string, content, group_id, datetime);
					
                    currentNotificationIndex = currentNotificationIndex + 1 >= notificationsSended.capacity() 
                                                    ? 0 : currentNotificationIndex + 1;

					sendToEveryone(query);
					std::cout << "sended to everyone: " << query << std::endl;
				}
            }
		}
	});
	tick.detach();
    
    // Initilize app
    uWS::App()

	.get("/", [&fileStreamer](auto *res, auto *req) {
	    std::cout << "Index!" << std::endl;

        std::string rangeHeader(req->getHeader("range"));

        fileStreamer.streamFile(res, "/index.html", rangeHeader);
    })
    
    // make sure to copy currentNotificationIndex
    .get("/api/last_notifications", [&notificationsSended, currentNotificationIndex](auto* res, uWS::HttpRequest* req){
        res->writeHeader("Content-Type", "application/json");
        if (notificationsSended.size() > 0) {
            std::string lastNotifications = "[";
            const size_t start = currentNotificationIndex + 1  >= notificationsSended.capacity() 
                                    ? 0 : currentNotificationIndex + 1;
            size_t i = start;
            while(i != currentNotificationIndex) {
                if (!notificationsSended[i].empty()) {
                    lastNotifications += notificationsSended[i] + ",";
                }
                i = i + 1 >= notificationsSended.capacity() ? 0 : i + 1;
            }
            
            
            lastNotifications[lastNotifications.size() - 1] = ']';
            
            res->end(GetJsonString("last_notifications", GetJsonString({{"notifications", lastNotifications}}, false)));
        } else {
            res->end(GetJsonString("last_notifications", "[]"));
        }
    })

    .get("/api/configuration_files", [](auto* res, uWS::HttpRequest* req){
        res->writeHeader("Content-Type", "application/json");
        res->end(GetConfigurationsPathsJson({"../../recognize/build/", "./configurations/"}));        
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

                    mediaPath = fs::absolute(configurations.programConfig.imagesFolder).string();

                    if (recognize.Start(current_configurations, 
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
	    recognize.CloseAndJoin();

        // response is a json
        res->writeHeader("Content-Type", "application/json");

        res->end(GetAlertMessage(AlertStatus::OK, "Recognizer stopped"));
        std::cout << "Closed recognize" << std::endl;
    })

    .get("/api/camera_frame", [&cachedImages](auto *res, auto *req) {
        const int rotation = std::stoi(static_cast<std::string>(req->getQuery("rotation")));
        const std::string url(req->getQuery("url"));
        const std::string roi_s(req->getQuery("roi"));

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
                res->end(GetJsonString("camera_frame", GetJsonString("frame", "\"" + encoded + "\"")));
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
        const std::string file(req->getQuery("file"));
        const std::string copy_path(req->getQuery("copy_path"));

        std::string error;

        // response is a json
        res->writeHeader("Content-Type", "application/json");

        std::cout 	<< "File requested=" << file
                    << std::endl
                    << "Copied to: " << copy_path
                    << std::endl;
        
        // create directory if missing
        std::filesystem::path path { copy_path };
        std::filesystem::create_directories(path.parent_path());
        
        // try to copy the file
        try {
            fs::copy_file(file, copy_path);
        } catch (std::filesystem::filesystem_error const& ex) {
            error = ex.what();
        }
        
        if (error.length() == 0) {
            // read the file
            Configurations cfgs = ConfigurationFile::ReadConfigurations(copy_path, error);
            if (error.length() == 0) {
                std::string stringCfgs = ConfigurationFile::ConfigurationsToString(cfgs);

                // send configuration
                res->end(GetJsonString("configuration_file", Json::Value(stringCfgs).toStyledString()));
            } else {
                res->end(GetAlertMessage(AlertStatus::ERROR, "The copied file was invalid and now you have 2 invalid files", error));
            }
        } else {
            res->end(GetAlertMessage(AlertStatus::ERROR, "Could not copy file", error));
        }
    })

    .get("/api/notifications", [&persintent_notifications](auto *res, auto *req) {
        // response is a json
        res->writeHeader("Content-Type", "application/json");
	    res->end(GetJsonString("notifications", persintent_notifications.toStyledString()));
    })

    .get("/media/:media", [](auto *res, auto *req) {
        // TODO:
        //  1.  Read all the files from lastMediaPath and update
        //      a map and update the missing ones, the key would
        //      be the filename with extension without path
        //
        //  2.  Search for a file named req->getParameter(0) and
        //      if found send it chunked, else send 404 Status.
        //
        //  When the server send a notification file first would 
        //  need to change the path to /media/filename, then 
        //  send the notification
        req->setYield(true);
    })

    // example for an async response
    .get("/api/example_async", [](auto *res, auto *req) {
        // move response and request to the new thread.
        std::thread threadReponse([r = res, rq = req]() {
            int total = 10;
            while(total--) {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }

            r->end("It's working!");
            std::cout << "Sended after 5 seconds!" << std::endl;
        });

        res->onAborted([]() {
            std::cout << "ABORTED! async." << std::endl;
        });
        
        threadReponse.detach();
    })

	.get("/*.*", [&fileStreamer](auto *res, auto *req) {
        std::string url(req->getUrl());
        std::string rangeHeader(req->getHeader("range"));

        if (!hasExtension(url)) {
            req->setYield(true); // mark as not handled
        } else if (fileStreamer.streamFile(res, url, rangeHeader)){
            // std::cout << "Succesfull sended file" << std::endl;
        } else {
            res->end();
        }
    })

    .get("/*", [](auto *res, auto *req) {
        res->writeStatus(HTTP_301_MOVED_PERMANENTLY);
        res->writeHeader("Location", "/");
        res->writeHeader("Content-Type", "text/html");
        res->end();
    })

    .ws<PerSocketData>("/recognize", {
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
        .open = [&clients](auto* ws) {
            /* Open event here, you may access ws->getUserData() which points to a PerSocketData struct */
            clients.push_back(ws);
        },
        .message = [&clients](auto *ws, std::string_view message, uWS::OpCode opCode) {
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
            // Test: clients.erase(ws);
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
