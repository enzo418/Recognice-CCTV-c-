#include "notification.hpp"


Notification::Notification::Notification(cv::Mat& img, std::string caption, bool save) : image(img), text(caption)  {
	this->type = Type::IMAGE;

	if (save) 
		this->filename = Utils::GetTimeFormated() + ".jpg";	
}

Notification::Notification::Notification(std::string text) : text(text) {
	this->type = Type::TEXT;
}

Notification::Notification::Notification() {
	this->type = Type::SOUND;
}

void Notification::Notification::send(ProgramConfiguration& programConfig) {
	if (this->type == Type::IMAGE && this->image.rows != 0) {
		if (programConfig.telegramConfig.useTelegramBot) {
			std::string location = programConfig.imagesFolder + "/" + this->filename;
			cv::imwrite("./" + location, this->image);

			TelegramBot::SendMediaToChat(location, this->text, programConfig.telegramConfig.chatId, programConfig.telegramConfig.apiKey);
		}
	} else if (this->type == Type::TEXT) {
		if (programConfig.telegramConfig.useTelegramBot) {
			TelegramBot::SendMessageToChat(this->text, programConfig.telegramConfig.chatId, programConfig.telegramConfig.apiKey);
		}
	}
}
