#include "seasocks/PageHandler.h"
#include "seasocks/PrintfLogger.h"
#include "seasocks/Server.h"
#include "seasocks/WebSocket.h"
#include "seasocks/StringUtil.h"

#include <opencv2/core.hpp>

#include "../../recognize/src/recognize.hpp"
#include "../../recognize/src/configuration_file.hpp"
#include "../../recognize/src/notification.hpp"
#include "../../recognize/src/utils.hpp"

#include "AreaSelector.hpp"

#include "base64.hpp"

#include <algorithm>
#include <memory>
#include <set>
#include <string>
#include <sstream>
#include <thread>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <map>
#include <tuple>

#include <fmt/core.h>
#include <fmt/color.h>

#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;
using namespace seasocks;

enum AlertStatus {
	ERROR 	= 0,
	OK 		= 1
};

std::string GetConfigurationsPaths(const std::vector<std::string>& directoriesToSeach);
std::string GetConfigurationsPathsJson(const std::vector<std::string>& directoriesToSeach);
std::string GetRecognizeStateJson();
std::string GetMediaPath();

std::string GetJsonString(const std::string& key, const std::string& value) {
	return fmt::format("{{\"{0}\": {1}}}", key, value);
}

/** Transform
 * { {"key1", "val1"}, {"key2", "val2"}, ...} => 
 * => {"key1": "val1", "key2": "val2", ...]
 * 
*/
std::string GetJsonString(const std::vector<std::pair<std::string, std::string>>& v) {
	std::string res = "{";
	for (auto &&i : v) {
		res += fmt::format("\"{}\": \"{}\",", i.first, i.second);
	}
	
	res.pop_back(); // pop last ,
	res += "}";

	return res;
}

std::string GetJsonString(const std::vector<std::pair<std::string, std::string>>& v, bool whitoutQuote) {
	std::string res = "{";
	for (auto &&i : v) {
		res += fmt::format("\"{}\": {},", i.first, i.second);
	}
	
	res.pop_back(); // pop last ,
	res += "}";

	return res;
}

/**
 * @brief Formats a alert message
 * @param status status of the alert, ok or error
 * @param message message of the alert
 * @param trigger_query query that triggered the alert, should be the same as the query received
 */
std::string GetAlertMessage(const AlertStatus& status, const std::string& message, const std::string& trigger_query  = "", const std::string& extra = "") {
	std::string st = AlertStatus::OK == status ? "ok" : "error";

	return GetJsonString("request_reply", GetJsonString({{"status", st}, {"message", message}, {"trigger", trigger_query}, {"extra", extra}}));
}

bool recognize_running = false;
size_t connections_number = 0;
std::map<std::string, std::string> connection_file;
std::vector<std::string> lastNotificationsSended;
std::map<std::string, std::string> cachedImages; // map of single images from the cameras in cache
Configurations current_configurations;

const std::string SERVER_FILEPATH = "../src/web";

const std::string ROOT_CONFIGURATIONS_DIRECTORY = "./configurations/";

std::string lastMediaPath = "";

constexpr int Max_Notifications_Number = 20;

namespace {
	struct HandlerFile : WebSocket::Handler {
		std::set<WebSocket*> _cons;
		Server* server;
		Recognize* recognize;

		HandlerFile(Server* server, Recognize* recognize) 
			: server(server), recognize(recognize) { };

		void onConnect(WebSocket* con) override {
			_cons.insert(con);
			con->send(GetConfigurationsPathsJson({"../../recognize/build/", "./configurations/"}));
			con->send(GetRecognizeStateJson());
			if (lastNotificationsSended.size() > 0) {
				std::string lastNotifications = "[";
				for (auto &&i : lastNotificationsSended) {
					lastNotifications += i + ",";
				}
				
				lastNotifications[lastNotifications.size() - 1] = ']';
				
				con->send(GetJsonString("last_notifications", GetJsonString({{"notifications", lastNotifications}}, false)));
			}

			con->send(GetJsonString("root_configurations_directory", "\""+ ROOT_CONFIGURATIONS_DIRECTORY + "\""));

			connections_number++;
		}

		void onDisconnect(WebSocket* con) override {
			_cons.erase(con);
			connections_number--;
			connection_file.erase(con->credentials()->username);
		}

		void onData(WebSocket* con, const char* data) override {
			Json::Value root;
			
			bool requestIsValid = false;

			Json::CharReaderBuilder reader;
			std::istringstream s(data);
			std::string errs;
			std::string id;

			if (Json::parseFromStream(reader, s, &root, &errs)) {
				if (root.isMember("key")) {					
					id = root["key"].asString();
					requestIsValid = true;
				}	
			}

			if (requestIsValid) {
				if (id == "need_config_file") {
					const std::string file = root["file"].asString();
					const bool isNew = root["is_new"].asBool();
					std::cout << "File requested=" << file << std::endl;
					std::string error;

					// read file
					Configurations cfgs = ConfigurationFile::ReadConfigurations(file, error);

					if (error.length() == 0) {
						std::string res = ConfigurationFile::ConfigurationsToString(cfgs);

						if (isNew) {
							std::filesystem::path path { file };
							
							std::cout << "\tFile is new. Full path: " << path << std::endl;

							std::filesystem::create_directories(path.parent_path());

							ConfigurationFile::SaveConfigurations(cfgs, path.string());
						}

						// send payload
						con->send(GetJsonString("configuration_file", Json::Value(res).toStyledString()));

						connection_file.insert(std::pair<std::string, std::string>(con->credentials()->username, file));
					} else {
						con->send(GetAlertMessage(AlertStatus::ERROR, "File could not be read, there is an invalid field", id, error));
					}
				} else if (id == "save_into_config_file") {
					std::string file = connection_file[con->credentials()->username];
					const std::string cfgString = root["configurations"].asString();
					std::cout 
						<< "File=" << file
						<< "Configuration size:\n" << cfgString.length() << std::endl;
					
					std::string error;
					std::istringstream iss(cfgString);
					Configurations configurations = ConfigurationFile::ReadConfigurationBuffer(iss, error);

					if (error.length() == 0) {
						std::cout << "There is " << configurations.camerasConfigs.size() << " cameras in the string\n";

						ConfigurationFile::SaveConfigurations(configurations, file);

						con->send(GetAlertMessage(AlertStatus::OK, "File saved correctly"));
					} else {
						con->send(GetAlertMessage(AlertStatus::ERROR, "File could not be saved, there is an invalid field", id, error));
					}
				} else if (id == "change_recognize_state") {
					bool success = true;

					// was stopped, run it
					if (!recognize_running) {
						std::string file = connection_file[con->credentials()->username];
						std::cout << "Starting recognize with file: " << file << std::endl;
						std::string error;
						Configurations configurations = ConfigurationFile::ReadConfigurations(file, error);
						
						if (error.length() == 0) {
							current_configurations = configurations;

							std::cout << "Config cameras size: " << configurations.camerasConfigs.size() << std::endl;
	
							fs::create_directories(configurations.programConfig.imagesFolder);

							lastMediaPath = configurations.programConfig.imagesFolder.substr(
												SERVER_FILEPATH.size(), 
												configurations.programConfig.imagesFolder.size()
											);

							if (recognize->Start(std::move(configurations), 
												configurations.programConfig.showPreview, 
												configurations.programConfig.telegramConfig.useTelegramBot)) {							
								con->send(GetAlertMessage(AlertStatus::OK, "Recognizer started"));
							} else {
								success = false;
								con->send(GetAlertMessage(AlertStatus::ERROR, "Could not start the recognizer, check that the configuration file has active cameras.", id, error));
							}
						} else {
							success = false;
							con->send(GetAlertMessage(AlertStatus::ERROR, "File could not be read, there is an invalid field", id, error));
						}
					} else { // was running, stop it
						recognize->CloseAndJoin();

						con->send(GetAlertMessage(AlertStatus::OK, "Recognizer stopped"));
						std::cout << "Closed recognize" << std::endl;
					}

					if (success) {
						recognize_running = !recognize_running;					
						sendEveryone(GetRecognizeStateJson());
					}
				} else if (id == "get_camera_frame") {
					const unsigned int index = root["index"].asUInt();
					const int rotation = root["rotation"].asInt();
					const std::string url = root["url"].asString();
					std::string roi_s;
					if (root.isMember("roi")) {
						roi_s = root["roi"].asString();
					}

					if (!url.empty()) {
						std::string cacheKey = url + std::to_string(rotation) + roi_s;
						std::string encoded;
						bool error = false;
						if (cachedImages.find(cacheKey) == cachedImages.end()) {							
							cv::Mat img;
							if (AreaSelector::GetFrame(url, img)) {		
								AreaSelector::ResizeRotateFrame(img, rotation);

								if (!roi_s.empty()) {
									const std::vector<int> numbers = Utils::GetNumbersString(roi_s);
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
								con->send(GetAlertMessage(AlertStatus::ERROR, "Could not open a connection to the camera", id));
								error = true;
							}
						} else {
							encoded = cachedImages[cacheKey];
						}

						if (!error)
							con->send(GetJsonString("frame_camera", GetJsonString({{"camera", std::to_string(index)},{"frame", encoded}})));
					} else {
						con->send(GetAlertMessage(AlertStatus::ERROR, "The camera url is empty", id));
					}
				} else if (id == "get_new_camera") {
					CameraConfiguration cfg;
					Json::Value root;
					root["new_camera_config"]["configuration"] = Json::Value(ConfigurationFile::GetConfigurationString(cfg));
					con->send(root.toStyledString());
				}  else if (id == "need_copy_file") {
					const auto file = root["file"].asString();
					const std::string copy_path = root["copy_path"].asString();

					std::cout 	<< "File requested=" << file
								<< std::endl
								<< "Copied to: " << copy_path
								<< std::endl;
					
					// create directory
					std::filesystem::path path { copy_path };
					std::filesystem::create_directories(path.parent_path());
					
					// copy file
					fs::copy_file(file, copy_path);

					// read file
					std::string error;
					Configurations cfgs = ConfigurationFile::ReadConfigurations(copy_path, error);
					if (error.length() == 0) {
						std::string res = ConfigurationFile::ConfigurationsToString(cfgs);

						// send payload
						con->send(GetJsonString("configuration_file", Json::Value(res).toStyledString()));

						connection_file.insert(std::pair<std::string, std::string>(con->credentials()->username, copy_path));
					} else {
						con->send(GetAlertMessage(AlertStatus::ERROR, "The copied file was invalid and now you have 2 invalid files", id, error));
					}
				} else {
					std::cout << "Command without handler received: '" << id << "'\n";
				}
			} else {
				fmt::print(fmt::emphasis::bold | fg(fmt::color::red), "ERROR: ");
				fmt::print("Couldn't parse string: {}\n", data);
			}
		}

		void sendEveryone(const std::string& msg) {
			for (auto* con : _cons) {
				con->send(msg);
			}
		}

		void Tick() {
			server->execute([this] {
				std::tuple<Notification::Type, std::string, ulong> media;
				while (recognize->notificationWithMedia->try_dequeue(media)) {					
					std::string query = "";
					Notification::Type type; std::string content; ulong group_id;

					std::tie(type, content, group_id) = media;

					if (type == Notification::IMAGE || type == Notification::GIF || type == Notification::VIDEO) {
						query = type == Notification::VIDEO ? "video" : "image";
						
						std::size_t found = content.find_last_of("/\\");
						content = lastMediaPath + "/" + content.substr(found+1);
					} else if (type == Notification::TEXT)
						query = "text";
					else if (type == Notification::SOUND)
						query = "sound";

					const std::string body = fmt::format("{{\"type\":\"{0}\", \"content\":\"{1}\", \"group\":\"{2}\"}}", query, content, group_id);
					query = fmt::format("{{\"new_notification\": {}}}", body);

					if (lastNotificationsSended.size() > Max_Notifications_Number)
						lastNotificationsSended.erase(lastNotificationsSended.begin());
						
					lastNotificationsSended.push_back(body);
					
					sendEveryone(query);
					std::cout << "sended to everyone: " << query << std::endl;
				}
			});
		}
	};

	struct MyAuthHandler : PageHandler {
		std::shared_ptr<Response> handle(const Request& request) override {
			request.credentials()->username = std::to_string(connections_number);
			return Response::unhandled(); // cause next handler to run
		}
	};
}

int main(int /*argc*/, const char* /*argv*/[]) {
	Recognize recognize;

	const uint16_t port = 9000;
	Server server(std::make_shared<PrintfLogger>(Logger::Level::Error));

	server.addPageHandler(std::make_shared<MyAuthHandler>());
	std::shared_ptr<HandlerFile> handler = std::make_shared<HandlerFile>(&server, &recognize);
	server.addWebSocketHandler("/file", handler);
	
	std::cout << "Listening in: http://localhost:" << port << std::endl;

	std::thread tick([&] {
		for(;;) {
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
			if (recognize_running)
				handler->Tick();
		}
	});
	tick.detach();

	server.serve(SERVER_FILEPATH.c_str(), port);
	return 0;
}

std::string GetConfigurationsPaths(const std::vector<std::string>& directoriesToSeach) {
	std::string configsFiles = "[";

	size_t i = 0;
	for (const auto& directory : directoriesToSeach) {
		if (fs::exists(directory)) {
			for (const auto & entry : std::filesystem::directory_iterator(directory)) {
				std::string path = entry.path().generic_string();
				if (entry.path().extension() == ".ini") {
					if(i > 0) configsFiles += ",";

					configsFiles += "\"" + path + "\"";
					i++;
				}
			}
		}
	}

	configsFiles += "]";
	
	return configsFiles;
}

std::string GetConfigurationsPathsJson(const std::vector<std::string>& directoriesToSeach) {
	return GetJsonString("configuration_files", GetConfigurationsPaths(directoriesToSeach));
}

std::string GetRecognizeStateJson() {
	return GetJsonString("recognize_state_changed", recognize_running ? "true" : "false");
}
