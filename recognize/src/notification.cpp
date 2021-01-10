#include "notification.hpp"

namespace Notification {
	Notification::Notification(cv::Mat& img, std::string caption, bool save) : image(img), text(caption)  {
		this->type = Type::IMAGE;

		if (save) 
			this->filename = Utils::GetTimeFormated() + ".jpg";	
	}

	Notification::Notification(std::string text) : text(text) {
		this->type = Type::TEXT;
	}

	Notification::Notification() {
		this->type = Type::SOUND;
	}

	std::string Notification::send(ProgramConfiguration& programConfig) {
		if (this->type == Type::IMAGE && this->image.rows != 0) {			
			std::string location = programConfig.imagesFolder + "/" + this->filename;
			cv::imwrite("./" + location, this->image);

			if (programConfig.telegramConfig.useTelegramBot)
				TelegramBot::SendMediaToChat(location, this->text, programConfig.telegramConfig.chatId, programConfig.telegramConfig.apiKey);	

			return location;	
		} else if (this->type == Type::TEXT) {
			if (programConfig.telegramConfig.useTelegramBot) {
				TelegramBot::SendMessageToChat(this->text, programConfig.telegramConfig.chatId, programConfig.telegramConfig.apiKey);				
			}
			return this->text;
		} else {
			return "sound";
		}
	}

	std::string Notification::getString() {
		if (this->type == Type::IMAGE)
			return this->filename;			
		else if (this->type == Type::TEXT)
			return this->text;
		else
			return "SOUND";
	}
}