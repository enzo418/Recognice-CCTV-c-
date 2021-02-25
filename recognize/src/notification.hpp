#pragma once
#include <opencv2/opencv.hpp>
#include "utils.hpp"
#include "types_configuration.hpp"
#include "telegram_bot.hpp"

namespace Notification {
	enum Type {SOUND = 0, TEXT, IMAGE, GIF};

	class Notification {
		private:
			cv::Mat image;
			std::string text;
			std::string filename = "test.jpg";
			std::string build_media_command;
			
			// path to the video that trigger the notification
			std::string videoPath;
			
		public:
			Type type;

			// Creates a image + text notification
			Notification(cv::Mat& image, std::string caption, const std::string& videoPath, bool save = false);

			// Creates a Gif notificatoin with caption + a command to build the media
			Notification(std::string mediaPath, std::string caption, std::string build_command, const std::string& videoPath);
			
			// Creates a text notification
			Notification(std::string text, const std::string& videoPath);
			
			// Creates a sound notification
			Notification();

			std::string send(ProgramConfiguration& programConfig);

			std::string getString();

			std::string getVideoPath();

			void buildMedia();
	};
}

// std::ostream& operator<<(std::ostream& out, Notification::Notification const& n) {
// 	out << "Type " << n.type;
// 	return out;
// }