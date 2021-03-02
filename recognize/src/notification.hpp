#pragma once
#include <opencv2/opencv.hpp>
#include "utils.hpp"
#include "types_configuration.hpp"
#include "telegram_bot.hpp"

namespace Notification {
	enum Type {SOUND = 0, TEXT, IMAGE, GIF, VIDEO};

	class Notification {
		private:
			cv::Mat image;
			std::string text;
			std::string filename = "test.jpg";
			std::string build_media_command;
			
			// if a notification is sended with others, then use this id to group them
			ulong group_id;
			
		// private:
		public:
			// Creates a image + text notification
			Notification(cv::Mat& image, std::string caption, bool save, ulong group_id = 0);

			// Creates a Gif notificatoin with caption + a command to build the media
			Notification(std::string mediaPath, std::string caption, std::string build_command, ulong group_id = 0);
			
			// Creates a Video notification with caption
			Notification(std::string mediaPath, std::string caption, ulong group_id = 0);
			
			// Creates a text notification
			Notification(std::string text, ulong group_id = 0);
			
			// Creates a sound notification
			Notification();

		public:
			// named constructors idiom
			static Notification Image(cv::Mat& image, std::string caption, bool save, ulong group_id = 0);

			static Notification Gif(std::string mediaPath, std::string caption, std::string build_command, ulong group_id = 0);

			static Notification Video(std::string mediaPath, std::string caption, ulong group_id = 0);

			static Notification Text(std::string text, ulong group_id = 0);

			static Notification Sound();
		
		public:
			Type type;

			bool sended;

			std::string send(ProgramConfiguration& programConfig);

			std::string getString();

			ulong getGroupId();

			void buildMedia();
	};
}

// std::ostream& operator<<(std::ostream& out, Notification::Notification const& n) {
// 	out << "Type " << n.type;
// 	return out;
// }