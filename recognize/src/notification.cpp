#include "notification.hpp"

namespace Notification {
	
	// ---------------
	//  Constructors
	// ---------------
	Notification::Notification(cv::Mat& img, std::string caption, bool save, ulong videoPath) 
		: 	image(img), 
			text(caption),
			group_id(group_id)
	{
		this->type = Type::IMAGE;

		if (save) 
			this->filename = Utils::GetTimeFormated() + ".jpg";	
	}

	Notification::Notification(std::string mediaPath, std::string caption, std::string build_command, ulong group_id) 
		: 	filename(mediaPath), 
			text(caption),
			build_media_command(build_command),
			group_id(group_id)
	{
		this->type = Type::GIF;
	}

	Notification::Notification(std::string mediaPath, std::string caption, ulong group_id) 
		: 	filename(mediaPath), 
			text(caption),
			group_id(group_id)
	{
		this->type = Type::VIDEO;
	}

	Notification::Notification(std::string text, ulong group_id) 
		: 	text(text),
			group_id(group_id)
	{
		this->type = Type::TEXT;
	}

	Notification::Notification() {
		this->type = Type::SOUND;
	}

	// --------------------------
	//  Named contructors idiom
	// --------------------------
	inline Notification Notification::Image(cv::Mat& image, std::string caption, bool save, ulong group_id) {
		return Notification(image, caption, save, group_id);
	}

	inline Notification Notification::Gif(std::string mediaPath, std::string caption, std::string build_command, ulong group_id) {
		return Notification(mediaPath, caption, build_command, group_id);
	}

	inline Notification Notification::Video(std::string mediaPath, std::string caption, ulong group_id) {
		return Notification(mediaPath, caption, group_id);
	}

	inline Notification Notification::Text(std::string text, ulong group_id) {
		return Notification(text, group_id);
	}

	inline Notification Notification::Sound() {
		return Notification();
	}
	
	// ----------------
	//  Public Methods
	// ----------------
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
		}  else if (this->type == Type::VIDEO) {
			if (programConfig.telegramConfig.useTelegramBot
					&& programConfig.telegramConfig.sendVideoWhenDetectChange) {
				TelegramBot::SendMediaToChat(this->filename, this->text, programConfig.telegramConfig.chatId, programConfig.telegramConfig.apiKey, true);
			}
			return this->filename;
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
	
	ulong Notification::getGroupId() {
		return this->group_id;
	}

	void Notification::buildMedia() {
		std::system(this->build_media_command.c_str());
	}
}