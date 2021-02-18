#include "notification.hpp"

namespace Notification {
	Notification::Notification(cv::Mat& img, std::string caption, bool save) : image(img), text(caption)  {
		this->type = Type::IMAGE;

		if (save) 
			this->filename = Utils::GetTimeFormated() + ".jpg";	
	}

	Notification::Notification(std::string mediaPath, std::string caption, std::string build_command) 
		: 	filename(mediaPath), 
			text(caption),
			build_media_command(build_command) {
		this->type = Type::GIF;
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

			if (programConfig.telegramConfig.useTelegramBot 
					&& programConfig.telegramConfig.sendImageWhenDetectChange)
				TelegramBot::SendMediaToChat(location, this->text, programConfig.telegramConfig.chatId, programConfig.telegramConfig.apiKey);	

			return location;	
		} else if (this->type == Type::GIF) {
			if (programConfig.telegramConfig.useTelegramBot
					&& programConfig.telegramConfig.sendGifWhenDetectChange)
				TelegramBot::SendMediaToChat(this->filename, this->text, programConfig.telegramConfig.chatId, programConfig.telegramConfig.apiKey, true);
			
			return this->filename;
		} else if (this->type == Type::TEXT) {
			if (programConfig.telegramConfig.useTelegramBot
					&& programConfig.telegramConfig.sendTextWhenDetectChange) {
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

	void Notification::buildMedia() {
		std::system(this->build_media_command.c_str());
	}
}