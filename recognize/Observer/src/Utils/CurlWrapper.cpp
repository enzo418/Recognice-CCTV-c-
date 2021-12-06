#include "CurlWrapper.hpp"

#include <curl/easy.h>

#include <memory>

void parse_url_qparams(std::string& url, CurlWrapper::qparams& qparams) {
    if (!qparams.empty()) {
        url += "?";
        for (auto const& param : qparams) {
            url += param.first + "=" + param.second + "&";
        }
        url.pop_back();
    }
}

CurlWrapper::CurlWrapper() { this->curl_ = curl_easy_init(); }
CurlWrapper::~CurlWrapper() {
    if (this->curl_ != nullptr) {
        curl_easy_cleanup(this->curl_);
    }

    if (post_ != nullptr) {
        curl_formfree(post_);
    }

    if (this->headers_list_) {
        curl_slist_free_all(headers_list_);
    }
}

CurlWrapper& CurlWrapper::url(const std::string& url, bool ipv6) {
    this->ipv6_ = ipv6;
    this->url_ = url;

    return *this;
}

CurlWrapper& CurlWrapper::qparam(const std::string& param,
                                 const std::string& value) {
    this->qparams_.insert_or_assign(param, value);
    return *this;
}

CurlWrapper& CurlWrapper::header(const std::string& param,
                                 const std::string& value) {
    this->headers_.insert_or_assign(param, value);
    return *this;
}

CurlWrapper& CurlWrapper::body(const std::string& pBody) {
    this->body_ = pBody;
    return *this;
}

CurlWrapper& CurlWrapper::method(CURLoption method) {
    this->method_ = method;

    return *this;
}

curl_wrapper_response CurlWrapper::perform(bool customWrite) {
    // --- Set url
    parse_url_qparams(this->url_, this->qparams_);

    curl_easy_setopt(this->curl_, CURLOPT_URL, url_.c_str());

    if (!this->ipv6_) {
        // reduce dns time
        curl_easy_setopt(this->curl_, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
    }

    // -- Set headers
    if (!headers_.empty()) {
        for (auto const& param : headers_) {
            headers_list_ = curl_slist_append(
                headers_list_, (param.first + ": " + param.second).c_str());
        }

        curl_easy_setopt(this->curl_, CURLOPT_HTTPHEADER, headers_list_);
    }

    // delay method so we can ad the post as a valid pointer,
    // and not as nullptr
    if (this->method_ == CURLOPT_HTTPPOST) {
        curl_easy_setopt(this->curl_, CURLOPT_HTTPPOST, post_);

        if (!this->body_.empty()) {
            curl_easy_setopt(this->curl_, CURLOPT_POSTFIELDS,
                             this->body_.c_str());
        }
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
    auto curl_code = curl_easy_perform(this->curl_);
    curl_easy_getinfo(this->curl_, CURLINFO_RESPONSE_CODE, &httpCode);

    return curl_wrapper_response(httpCode, httpData,
                                 curl_code != CURLE_ABORTED_BY_CALLBACK);
}

CurlWrapper& CurlWrapper::formAdd(const std::string& name, int value_content,
                                  const std::string& value) {
    curl_formadd(&this->post_, &this->last_, CURLFORM_COPYNAME, name.c_str(),
                 value_content, value.c_str(), CURLFORM_END);
    return *this;
}

CURL* CurlWrapper::get() { return this->curl_; }

curl_httppost* CurlWrapper::getPost() { return this->post_; }
