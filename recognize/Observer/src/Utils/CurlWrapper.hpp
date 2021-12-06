#pragma once

#include <curl/curl.h>

#include <map>
#include <string>
#include <string_view>

namespace {
    inline std::size_t callback(const char* in, std::size_t size,
                                std::size_t num, std::string* out) {
        const std::size_t totalBytes(size * num);
        out->append(in, totalBytes);
        return totalBytes;
    }
}  // namespace

struct curl_wrapper_response {
    curl_wrapper_response(int pCode, const std::string& pContent,
                          bool pCurl_sucess)
        : code(pCode), content(pContent), curl_sucess(pCurl_sucess) {}

    bool isDone() { return this->curl_sucess; }

    int code;
    std::string content;

   private:
    // wether curl could send it
    bool curl_sucess;
};

class CurlWrapper {
   public:
    CurlWrapper();
    ~CurlWrapper();

    CurlWrapper& url(std::string url, bool ipv6 = false);

    CurlWrapper& qparam(std::string param, std::string value);

    CurlWrapper& header(std::string param, std::string value);

    CurlWrapper& body(std::string body);

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

    using qparams = std::map<std::string, std::string>;
    using headers = std::map<std::string, std::string>;

   private:
    CURL* curl_ = nullptr;
    struct curl_httppost* post_ = nullptr;
    struct curl_httppost* last_ = nullptr;
    struct curl_slist* headers_list_ = NULL;

   private:
    qparams qparams_;
    headers headers_;

   private:
    CURLoption method_;
    std::string url_;
    bool ipv6_;
    std::string body_;

   private:
    void build_url_with_qparams();
    std::string encode_url(const std::string&);
};

template <typename... Args>
CurlWrapper& CurlWrapper::setOpt(CURLoption option, Args&&... args) {
    curl_easy_setopt(this->curl_, option, args...);
    return *this;
}