#pragma once

#include <curl/curl.h>

#include <string>
#include <string_view>

namespace {
    std::size_t callback(const char* in, std::size_t size, std::size_t num,
                         std::string* out) {
        const std::size_t totalBytes(size * num);
        out->append(in, totalBytes);
        return totalBytes;
    }
}  // namespace

struct curl_wrapper_response {
    curl_wrapper_response(int pCode, const std::string& pContent)
        : code(pCode), content(pContent) {}
    int code;
    std::string content;
};

class CurlWrapper {
   public:
    CurlWrapper();
    ~CurlWrapper();

    CurlWrapper& url(const std::string& url, bool ipv6 = false);
    CurlWrapper& method(CURLoption method);

    template <typename... Args>
    CurlWrapper& setOpt(CURLoption option, Args&&... args);

    /**
     * @brief Adds a field to the form.
     *
     * @param name name of the field
     * @param value_content CURLFORM_FILE, CURLFORM_COPYCONTENTS, etc
     * @param value value of the field
     */
    CurlWrapper& formAdd(const std::string& name, int value_content,
                         const std::string& value);

    /**
     * @brief Perfomns the request.
     *
     * @param customWrite if false, before calling this function, you will need
     * to set the opts CURLOPT_WRITEFUNCTION and CURLOPT_WRITEDATA to read the
     * response (optional).
     * @return curl_wrapper_response Response, empty if customWrite is true
     */
    curl_wrapper_response perform(bool customWrite = false);

    /**
     * @brief Returns curl instance.
     * Preferably don't use it.
     * @return CURL*
     */
    CURL* get();
    curl_httppost* getPost();

   private:
    CURL* curl_ = nullptr;
    struct curl_httppost* post_ = nullptr;
    struct curl_httppost* last_ = nullptr;
    CURLoption method_;
};

template <typename... Args>
CurlWrapper& CurlWrapper::setOpt(CURLoption option, Args&&... args) {
    curl_easy_setopt(this->curl_, option, args...);
    return *this;
}