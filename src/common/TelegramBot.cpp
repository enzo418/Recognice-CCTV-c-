#include "TelegramBot.hpp"

namespace
{
    std::size_t callback(
            const char* in,
            std::size_t size,
            std::size_t num,
            std::string* out)
    {
        const std::size_t totalBytes(size * num);
        out->append(in, totalBytes);
        return totalBytes;
    }
}

std::string GetLastMessageFromBot(std::string& apiKey, std::string& result, std::time_t& unixTimeMs, const std::vector<std::string>& authUsers) {
	const std::string url("https://api.telegram.org/bot" + apiKey + "/getUpdates?offset=-1");
		
	result = "";
	std::string senderID = "";
	
	CURL* curl = curl_easy_init();

	if(curl) {
		// Set remote URL.
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		// Don't bother trying IPv6, which would increase DNS resolution time.
		curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

		// Don't wait forever, time out after 10 seconds.
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

		// Follow HTTP redirects if necessary.
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		// Response information.
		long httpCode(0);
		std::unique_ptr<std::string> httpData(new std::string());

		// Hook up data handling function.
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);

		// Hook up data container (will be passed as the last parameter to the
		// callback handling function).  Can be any pointer type, since it will
		// internally be passed as a void pointer.
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

		// Run our HTTP GET command, capture the HTTP response code, and clean up.
		curl_easy_perform(curl);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
		curl_easy_cleanup(curl);

		if (httpCode == 200) {
			// Got successful response

			Json::Value jsonData;
			Json::Reader jsonReader;

			if (jsonReader.parse(*httpData.get(), jsonData)) {
				// Successfully parsed JSON data

				if(jsonData["result"].size() > 0) {
					jsonData = jsonData["result"][0]["message"];
					Json::Value def = Json::Value(0);

					unixTimeMs = jsonData["date"].asUInt64();

					senderID = jsonData["from"]["id"].asString();

					const std::string username = jsonData["from"]["username"].asString();
					
					if(!ExistInVector(authUsers, username)) {
						std::cout << "User is'nt auth: " << username << std::endl;
						return "";
						// ? return or call again
					}
					
					Json::Value text = jsonData.get("text", def);
					if(text != def) {
						// The message has text
						result = text.asString();
					}
					
					Utils::toLowerCase(result);
				}
			}  else {
				std::cout << "Could not parse HTTP data as JSON" << std::endl;
				std::cout << "HTTP data was:\n" << *httpData.get() << std::endl;            
			}
		} else {
			std::cout << "Couldn't GET from " << url << " - exiting" << std::endl;        
		}
	}
	
	return senderID;
}

void SendMessageToChat(const std::string& message, std::string& chatID, std::string& apiKey) {
	std::string url("https://api.telegram.org/bot" + apiKey + "/sendMessage");
	
	CURL* curl = curl_easy_init();

	if(curl){ 
		// Set data (it's easier to add it to the url)
		url += "?chat_id=" + chatID + "&text="+ message;

		// Set remote URL.
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		// Don't bother trying IPv6, which would increase DNS resolution time.
		curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

		// Don't wait forever, time out after 10 seconds.
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

		// Follow HTTP redirects if necessary.
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		// Response information.
		long httpCode(0);
		std::unique_ptr<std::string> httpData(new std::string());

		// Hook up data handling function.
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);

		// Hook up data container (will be passed as the last parameter to the
		// callback handling function).  Can be any pointer type, since it will
		// internally be passed as a void pointer.
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

		// Run our HTTP GET command, capture the HTTP response code, and clean up.
		curl_easy_perform(curl);
		curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
		curl_easy_cleanup(curl);

		if(httpCode != 200) std::cout << "Couldn't send the message. Http code: " << httpCode << std::endl;
	}
}

void SendImageToChat(const std::string& imagePath, const std::string& caption, std::string& chatID, std::string& apiKey){
	std::string url("https://api.telegram.org/bot" + apiKey + "/sendphoto");

	CURL* curl = curl_easy_init();

	struct curl_httppost* post = NULL;
 	struct curl_httppost* last = NULL;

	if(curl){
		// Set remote URL.
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

		// Don't bother trying IPv6, which would increase DNS resolution time.
		curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);

		// Don't wait forever, time out after 10 seconds.
		curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10);

		// Follow HTTP redirects if necessary.
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);

		// Set up form
		// Add photo
		curl_formadd(&post, &last,
                 CURLFORM_COPYNAME, "photo",
                 CURLFORM_FILE, imagePath.c_str(),
                 CURLFORM_END);
		
		// Add caption
		curl_formadd(&post, &last,
                 CURLFORM_COPYNAME, "caption",
                 CURLFORM_COPYCONTENTS, caption.c_str(),
                 CURLFORM_END);
		
		// Add chat_id
		curl_formadd(&post, &last,
                 CURLFORM_COPYNAME, "chat_id",
                 CURLFORM_COPYCONTENTS, chatID.c_str(),
                 CURLFORM_END);
		
		// set the post in curl
		curl_easy_setopt(curl, CURLOPT_HTTPPOST, post); 

		// Hook up data handling function.
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, callback);

		long httpCode(0);
		std::unique_ptr<std::string> httpData(new std::string());

		// Hook up data container (will be passed as the last parameter to the
		// callback handling function).  Can be any pointer type, since it will
		// internally be passed as a void pointer.
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, httpData.get());

		curl_easy_perform(curl);

		curl_easy_cleanup(curl);

		curl_formfree(post);
	}
}