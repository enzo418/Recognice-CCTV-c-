#include "CurlWrapper.hpp"

#include <curl/curl.h>
#include <curl/easy.h>

#include <memory>

CurlWrapper::CurlWrapper() {
    // In main() we call curl_global_init(CURL_GLOBAL_ALL);

    this->curl_ = curl_easy_init();
    if (!this->curl_) {
        OBSERVER_ERROR("Couldn't initialize curl.");
    }
}

CurlWrapper::~CurlWrapper() {
    if (this->curl_ != nullptr) {
        curl_easy_cleanup(this->curl_);
        this->curl_ = nullptr;
    }

    if (post_ != nullptr) {
        curl_formfree(post_);
    }

    if (this->headers_list_) {
        curl_slist_free_all(headers_list_);
    }
}

CurlWrapper& CurlWrapper::url(std::string url, bool ipv6) {
    this->ipv6_ = ipv6;
    this->url_ = url;

    return *this;
}

CurlWrapper& CurlWrapper::qparam(std::string param, std::string value) {
    this->qparams_.insert_or_assign(param, value);
    return *this;
}

CurlWrapper& CurlWrapper::header(std::string param, std::string value) {
    this->headers_.insert_or_assign(param, value);
    return *this;
}

CurlWrapper& CurlWrapper::body(std::string pBody) {
    this->body_ = pBody;
    return *this;
}

CurlWrapper& CurlWrapper::method(CURLoption method) {
    this->method_ = method;

    return *this;
}

curl_wrapper_response CurlWrapper::perform(bool customWrite) {
    // --- Set url
    build_url_with_qparams();

    if (curl_easy_setopt(this->curl_, CURLOPT_URL, url_.c_str())) {
        OBSERVER_ERROR("Couldn't set the url.");
        return curl_wrapper_response(0, "", false);
    }

    if (!this->ipv6_) {
        // reduce dns time
        if (curl_easy_setopt(this->curl_, CURLOPT_IPRESOLVE,
                             CURL_IPRESOLVE_V4)) {
            OBSERVER_ERROR("Couldn't set the ip resolve.");
        }
    }

    // -- Set headers
    if (!headers_.empty()) {
        for (auto const& param : headers_) {
            headers_list_ = curl_slist_append(
                headers_list_, (param.first + ": " + param.second).c_str());
        }

        if (curl_easy_setopt(this->curl_, CURLOPT_HTTPHEADER, headers_list_)) {
            OBSERVER_ERROR("Couldn't set the headers.");
        }
    }

    // delay method so we can ad the post as a valid pointer,
    // and not as nullptr
    if (this->method_ == CURLOPT_HTTPPOST) {
        if (curl_easy_setopt(this->curl_, CURLOPT_HTTPPOST, post_)) {
            OBSERVER_ERROR("Couldn't set the HTTP POST.");
        }

        if (!this->body_.empty()) {
            // this->body_ = this->encode_url(this->body_);
            wt.readptr = this->body_.c_str();
            wt.sizeleft = this->body_.size();
            /* Now specify we want to POST data */
            if (curl_easy_setopt(this->curl_, CURLOPT_POST, 1L)) {
                OBSERVER_ERROR("Couldn't set the POST option.");
            }

            /* we want to use our own read function */
            if (curl_easy_setopt(this->curl_, CURLOPT_READFUNCTION,
                                 read_callback)) {
                OBSERVER_ERROR("Couldn't set the read callback.");
            }

            /* pointer to pass to our read function */
            if (curl_easy_setopt(this->curl_, CURLOPT_READDATA, &wt)) {
                OBSERVER_ERROR("Couldn't set the read data.");
            }

            if (curl_easy_setopt(this->curl_, CURLOPT_POSTFIELDSIZE,
                                 (long)wt.sizeleft)) {
                OBSERVER_ERROR("Couldn't set the POST field size.");
            }

            if (curl_easy_setopt(this->curl_, CURLOPT_VERBOSE, 1L)) {
                OBSERVER_ERROR("Couldn't set the verbose option.");
            }

            // curl_easy_setopt(this->curl_, CURLOPT_POSTFIELDS,
            //                  this->body_.c_str());
        }
    } else if (this->method_ == CURLOPT_HTTPGET) {
        if (curl_easy_setopt(this->curl_, CURLOPT_HTTPGET, 1L)) {
            OBSERVER_ERROR("Couldn't set the HTTP GET.");
        }
    } else {
        if (curl_easy_setopt(this->curl_, this->method_, 1L)) {
            OBSERVER_ERROR("Couldn't set the method.");
        }
    }

    // Response information.
    long httpCode(0);
    std::string httpData;

    if (!customWrite) {
        // Hook up data handling function.
        if (curl_easy_setopt(this->curl_, CURLOPT_WRITEFUNCTION,
                             write_callback)) {
            OBSERVER_ERROR("Couldn't set the write callback.");
        }

        // Hook up data container (will be passed as the last parameter to the
        // callback handling function).  Can be any pointer type, since it will
        // internally be passed as a void pointer.
        if (curl_easy_setopt(this->curl_, CURLOPT_WRITEDATA, &httpData)) {
            OBSERVER_ERROR("Couldn't set the write data.");
        }
    }

    // perform
    auto curl_code = curl_easy_perform(this->curl_);

    if (curl_easy_getinfo(this->curl_, CURLINFO_RESPONSE_CODE, &httpCode)) {
        if (curl_code == CURLE_READ_ERROR) {
            OBSERVER_WARN("Trying to send a image that doesn't exists.");
        }
    }

    if (curl_code != CURLE_OK) {
        OBSERVER_ERROR("Curl error: {}", curl_easy_strerror(curl_code));
    }

    return curl_wrapper_response(httpCode, httpData, curl_code == CURLE_OK);
}

CurlWrapper& CurlWrapper::formAdd(const std::string& name, int value_content,
                                  const std::string& value) {
    curl_formadd(&this->post_, &this->last_, CURLFORM_COPYNAME, name.c_str(),
                 value_content, value.c_str(), CURLFORM_END);
    return *this;
}

CURL* CurlWrapper::get() { return this->curl_; }

curl_httppost* CurlWrapper::getPost() { return this->post_; }

void CurlWrapper::build_url_with_qparams() {
    if (!this->qparams_.empty()) {
        if (this->url_.back() == '/') {
            this->url_.pop_back();
        }

        if (this->url_.back() != '?') {
            this->url_ += "?";
        }

        for (auto const& param : this->qparams_) {
            this->url_ +=
                param.first + "=" + this->encode_url(param.second) + "&";
        }
        this->url_.pop_back();
    }
}

std::string CurlWrapper::encode_url(const std::string& url) {
    return std::string(curl_easy_escape(this->curl_, url.c_str(), url.size()));
}
