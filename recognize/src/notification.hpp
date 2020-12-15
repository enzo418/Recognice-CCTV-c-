#pragma once
#include <opencv2/opencv.hpp>
#include "utils.hpp"
#include "types_configuration.hpp"
#include "telegram_bot.hpp"

namespace Notification {
	enum Type {SOUND = 0, TEXT, IMAGE};

	class Notification {
		private:
			cv::Mat image;
			std::string text;
			std::string filename = "test.jpg";
			
		public:
			Type type;

			// Creates a image + text notification
			Notification(cv::Mat& image, std::string caption, bool save = false);		
			
			// Creates a text notification
			Notification(std::string text);
			
			// Creates a sound notification
			Notification();

			void send(ProgramConfiguration& programConfig);
	};
}

// std::ostream& operator<<(std::ostream& out, Notification::Notification const& n) {
// 	out << "Type " << n.type;
// 	return out;
// }