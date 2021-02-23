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
			
		public:
			Type type;

			// Creates a image + text notification
			Notification(cv::Mat& image, std::string caption, bool save = false);

			// Creates a Gif notificatoin with caption + a command to build the media
			Notification(std::string mediaPath, std::string caption, std::string build_command);
			
			// Creates a text notification
			Notification(std::string text);
			
			// Creates a sound notification
			Notification();

			std::string send(ProgramConfiguration& programConfig);

			std::string getString();

			void buildMedia();
	};
}

// std::ostream& operator<<(std::ostream& out, Notification::Notification const& n) {
// 	out << "Type " << n.type;
// 	return out;
// }