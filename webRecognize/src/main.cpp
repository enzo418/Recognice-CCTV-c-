#include "seasocks/PageHandler.h"
#include "seasocks/PrintfLogger.h"
#include "seasocks/Server.h"
#include "seasocks/WebSocket.h"
#include "seasocks/StringUtil.h"

#include "../../recognize/src/recognize.hpp"
#include "../../recognize/src/configuration_file.hpp"
#include "../../recognize/src/notification.hpp"

#include <memory>
#include <set>
#include <string>
#include <thread>
#include <chrono>
#include <iostream>
#include <filesystem>
#include <json/json.h>
#include <map>

#include <fmt/core.h>

#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;
using namespace seasocks;

std::string GetConfigurationsPathsToJson();
std::string GetRecognizeStateJson();
std::string GetMediaPath();

bool recognize_running = false;
size_t connections_number = 0;
std::map<std::string, std::string> connection_file;

const std::string SERVER_FILEPATH = "../src/web";

std::string lastMediaPath = "";

namespace {
	struct HandlerFile : WebSocket::Handler {
		std::set<WebSocket*> _cons;
		Server* server;
		Recognize* recognize;

		HandlerFile(Server* server, Recognize* recognize) 
			: server(server), recognize(recognize) { };

		void onConnect(WebSocket* con) override {
			_cons.insert(con);
			con->send(GetConfigurationsPathsToJson());
			con->send(GetRecognizeStateJson());
			connections_number++;
		}
		void onDisconnect(WebSocket* con) override {
			_cons.erase(con);
			connections_number--;
			connection_file.erase(con->credentials()->username);
		}

		void onData(WebSocket* con, const char* data) override {
			const std::string string(data);

			// get command and value
			int indx = strcspn(data, " ");
			std::string id = string.substr(0, indx);
			std::string val = string.substr(indx + 1, string.size() - 1);

			if (id == "need_config_file") {
				std::cout << "File requested=" << val << std::endl;

				// read file
				Configurations cfgs = ConfigurationFile::ReadConfigurations(val);				
				std::string res = ConfigurationFile::ConfigurationsToString(cfgs);
				
				// prepare payload
				Json::Value root;
				Json::Value config_file = Json::Value(res);
				root["configuration_file"] = config_file;

				// send payload
				con->send(root.toStyledString());

				connection_file.insert(std::pair<std::string, std::string>(con->credentials()->username, val));
			} else if (id == "save_into_config_file") {
				std::string file = connection_file[con->credentials()->username];
				std::cout 
					<< "File=" << file
					<< "Configuration size:\n" << val.length() << std::endl;
				
				std::istringstream iss(val);
				Configurations configurations = ConfigurationFile::ReadConfigurationBuffer(iss);

				std::cout << "There is " << configurations.camerasConfigs.size() << " cameras in the string\n";

				ConfigurationFile::SaveConfigurations(configurations, file);
			} else if (id == "change_recognize_state") {
				recognize_running = !recognize_running;
				
				// was stopped, run it
				if (recognize_running) {
					std::string file = connection_file[con->credentials()->username];
					std::cout << "Starting recognize with file: " << file << std::endl;
					Configurations configurations = ConfigurationFile::ReadConfigurations(file);

					std::cout << "Config cameras size: " << configurations.camerasConfigs.size() << std::endl;

					fs::create_directories(configurations.programConfig.imagesFolder);

					lastMediaPath = configurations.programConfig.imagesFolder.substr(
										SERVER_FILEPATH.size(), 
										configurations.programConfig.imagesFolder.size()
									);

					recognize->Start(std::move(configurations), 
														configurations.programConfig.showPreview, 
														configurations.programConfig.telegramConfig.useTelegramBot);

				} else { // was running, stop it
					recognize->CloseAndJoin();

					std::cout << "Closed recognize" << std::endl;
				}

				sendEveryone(GetRecognizeStateJson());
			} else {
				std::cout << "Command without handler received: '" << id << " value=" << val << "'\n";
			}
		}

		void sendEveryone(const std::string& msg) {
			for (auto* con : _cons) {
				con->send(msg);
			}
		}

		// sample on how to implement a tick function
		void Tick() {
			server->execute([this] {
				std::pair<Notification::Type, std::string> media;
				while (recognize->notificationWithMedia->try_dequeue(media)) {					
					std::string command = "";

					if (media.first == Notification::IMAGE) {
						command = "image";
						std::size_t found = media.second.find_last_of("/\\");
						media.second = lastMediaPath + "/" + media.second.substr(found+1);
					} else if (media.first == Notification::TEXT)
						command = "text";
					else if (media.first == Notification::SOUND)
						command = "sound";

					command = fmt::format("{{\"new_notification\": {{\"type\":\"{0}\", \"content\":\"{1}\"}}}}", command, media.second);

					sendEveryone(command);
					std::cout << "sended to everyone: " << command << std::endl;
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

std::string GetConfigurationsPathsToJson() {
	Json::Value root;
	Json::Value configsFiles = Json::arrayValue;

	size_t i = 0;
	for (const auto & entry : std::filesystem::directory_iterator("../../recognize/build/")) {
		std::string path = entry.path().generic_string();
		if (entry.path().has_extension() 
			&& path.find(".ini") != std::string::npos
			&& path.find(".ini~") == std::string::npos) {
			Json::Value path_json = Json::Value(path);
			configsFiles.append(path_json);
			i++;
		}
	}

	root["configuration_files"] = configsFiles;

	return root.toStyledString();
}

std::string GetRecognizeStateJson() {
	Json::Value root;
	Json::Value state = Json::Value(recognize_running);

	root["recognize_state_changed"] = state;

	return root.toStyledString();
}
