#include "CurlWrapper.hpp"

#include <curl/easy.h>

#include <memory>

CurlWrapper::CurlWrapper() { this->curl_ = curl_easy_init(); }
CurlWrapper::~CurlWrapper() {
    if (this->curl_ != nullptr) {
        curl_easy_cleanup(this->curl_);
    }

    if (post_ != nullptr) {
        curl_formfree(post_);
    }
}

CurlWrapper& CurlWrapper::url(const std::string& url, bool ipv6) {
    curl_easy_setopt(this->curl_, CURLOPT_URL, url.c_str());

    if (!ipv6) {
        // reduce dns time
        curl_easy_setopt(this->curl_, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    }

    return *this;
}

CurlWrapper& CurlWrapper::method(CURLoption method) {
    this->method_ = method;

    return *this;
}

curl_wrapper_response CurlWrapper::perform(bool customWrite) {
    // delay method so we can ad the post as a valid pointer,
    // and not as nullptr
    if (this->method_ == CURLOPT_HTTPPOST) {
        curl_easy_setopt(this->curl_, CURLOPT_HTTPPOST, post_);
    } else {
        curl_easy_setopt(this->curl_, this->method_, 1l);
    }

    // Response information.
    long httpCode(0);
    std::string httpData;

    if (!customWrite) {
        // Hook up data handling function.
        curl_easy_setopt(this->curl_, CURLOPT_WRITEFUNCTION, callback);

        // Hook up data container (will be passed as the last parameter to the
        // callback handling function).  Can be any pointer type, since it will
        // internally be passed as a void pointer.
        curl_easy_setopt(this->curl_, CURLOPT_WRITEDATA, &httpData);
    }

    // perform
    curl_easy_perform(this->curl_);

    return curl_wrapper_response(httpCode, httpData);
}

CurlWrapper& CurlWrapper::formAdd(const std::string& name, int value_content,
                                  const std::string& value) {
    curl_formadd(&this->post_, &this->last_, CURLFORM_COPYNAME, name.c_str(),
                 value_content, value.c_str(), CURLFORM_END);
    return *this;
}

CURL* CurlWrapper::get() { return this->curl_; }

curl_httppost* CurlWrapper::getPost() { return this->post_; }
