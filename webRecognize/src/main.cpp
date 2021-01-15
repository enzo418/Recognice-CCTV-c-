#include "seasocks/PageHandler.h"
#include "seasocks/PrintfLogger.h"
#include "seasocks/Server.h"
#include "seasocks/WebSocket.h"
#include "seasocks/StringUtil.h"

#include "../../recognize/src/recognize.hpp"
#include "../../recognize/src/configuration_file.hpp"
#include "../../recognize/src/notification.hpp"

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
#include "base64.h"

#include <fmt/core.h>

#include <opencv2/core.hpp>

#include <cstdlib>
#include <filesystem>

namespace fs = std::filesystem;
using namespace seasocks;

std::string GetConfigurationsPaths(const std::vector<std::string>& directoriesToSeach);
std::string GetConfigurationsPathsJson(const std::vector<std::string>& directoriesToSeach);
std::string GetRecognizeStateJson();
std::string GetMediaPath();

std::string GetJsonString(const std::string& key, const std::string& value) {
	return fmt::format("{{\"{0}\": {1}}}", key, value);
}

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
			con->send(GetConfigurationsPathsJson({"../../recognize/build/"}));
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

				// send payload
				con->send(GetJsonString("configuration_file", Json::Value(res).toStyledString()));

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
				cv::Mat res;
				while (recognize->frames->try_dequeue(res)) {
					// Serialize the input image to a stringstream
					// std::stringstream serializedStream = serialize(res);
					// std::string serialized ((char*)res.data);
					std::vector<uchar> buf;
					cv::imencode(".jpg", res, buf);
					auto *enc_msg = reinterpret_cast<unsigned char*>(buf.data());
					std::string encoded = "\"" +  base64_encode(enc_msg, buf.size()) + "\"";


					// Base64 encode the std::stringstream
					// std::string encoded;
					// encoded = "\"" + Base64::Encode(serialized) + "\"";

					sendEveryone(GetJsonString("new_image", encoded));
				}
				

				// std::pair<Notification::Type, std::string> media;
				// while (recognize->notificationWithMedia->try_dequeue(media)) {					
				// 	std::string command = "";

				// 	if (media.first == Notification::IMAGE) {
				// 		command = "image";
				// 		std::size_t found = media.second.find_last_of("/\\");
				// 		media.second = lastMediaPath + "/" + media.second.substr(found+1);
				// 	} else if (media.first == Notification::TEXT)
				// 		command = "text";
				// 	else if (media.first == Notification::SOUND)
				// 		command = "sound";

				// 	command = fmt::format("{{\"new_notification\": {{\"type\":\"{0}\", \"content\":\"{1}\"}}}}", command, media.second);

				// 	sendEveryone(command);
				// 	std::cout << "sended to everyone: " << command << std::endl;
				// }
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

// Serialize a cv::Mat to a stringstream
std::stringstream serialize(cv::Mat input) {
       // We will need to also serialize the width, height, type and size of the matrix
       int width = input.cols;
       int height = input.rows;
       int type = input.type();
       size_t size = input.total() * input.elemSize();

       // Initialize a stringstream and write the data
       std::stringstream ss;
       ss.write((char*)(&width), sizeof(int));
       ss.write((char*)(&height), sizeof(int));
       ss.write((char*)(&type), sizeof(int));
       ss.write((char*)(&size), sizeof(size_t));

       // Write the whole image data
       ss.write((char*)input.data, size);

	return ss;
}

// Deserialize a Mat from a stringstream
cv::Mat deserialize(std::stringstream& input) {
       // The data we need to deserialize
       int width = 0;
       int height = 0;
       int type = 0;
       size_t size = 0;

       // Read the width, height, type and size of the buffer
       input.read((char*)(&width), sizeof(int));
       input.read((char*)(&height), sizeof(int));
       input.read((char*)(&type), sizeof(int));
       input.read((char*)(&size), sizeof(size_t));

       // Allocate a buffer for the pixels
       char* data = new char[size];
       // Read the pixels from the stringstream
       input.read(data, size);

       // Construct the image (clone it so that it won't need our buffer anymore)
       cv::Mat m = cv::Mat(height, width, type, data).clone();

       // Delete our buffer
       delete[]data;

       // Return the matrix
       return m;
}

int main(int /*argc*/, const char* /*argv*/[]) {
	Recognize recognize;

	const uint16_t port = 9001;
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
		for (const auto & entry : std::filesystem::directory_iterator(directory)) {
			std::string path = entry.path().generic_string();
			if (entry.path().extension() == ".ini") {
				if(i > 0) configsFiles += ",";

				configsFiles += "\"" + path + "\"";
				i++;
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
