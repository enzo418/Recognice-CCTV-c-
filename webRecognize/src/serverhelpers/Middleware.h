#pragma once

#include <string>
#include <uWebSockets/App.h>

/* Middleware to fill out content-type */
bool hasExtension(std::string_view file, std::string_view ext) {
    if (ext.size() > file.size()) {
        return false;
    }
    return std::equal(ext.rbegin(), ext.rend(), file.rbegin());
}

bool hasExtension(std::string_view url) {
    const std::string ext = ".";
    
    auto result = std::find_first_of(url.begin(), url.end(), ext.begin(), ext.end());

    return result != url.end();
}

/* This should be a filter / middleware like app.use(handler) */
template <bool SSL>
void setFileContentType(uWS::HttpResponse<SSL>* res, std::string_view req, const bool isRoot = false) {
    if (hasExtension(req, ".svg")) {
        res->writeHeader("Content-Type", "image/svg+xml");
    } else if(hasExtension(req, ".htm")) {
        res->writeHeader("Content-Type", "text/html");
    } else if(isRoot || hasExtension(req, ".html")) {
        res->writeHeader("Content-Type", "text/html");
    } else if(hasExtension(req, ".php")) {
        res->writeHeader("Content-Type", "text/html");
    } else if(hasExtension(req, ".css")) {
        res->writeHeader("Content-Type", "text/css");
    } else if(hasExtension(req, ".txt")) {
        res->writeHeader("Content-Type", "text/plain");
    } else if(hasExtension(req, ".js")) { 
        res->writeHeader("Content-Type", "application/javascript");
    } else if(hasExtension(req, ".json")) {
        res->writeHeader("Content-Type", "application/json");
    } else if(hasExtension(req, ".xml")) {
        res->writeHeader("Content-Type", "application/xml");
    } else if(hasExtension(req, ".swf")) {
        res->writeHeader("Content-Type", "application/x-shockwave-flash");
    } else if(hasExtension(req, ".flv")) {
        res->writeHeader("Content-Type", "video/x-flv");
    } else if(hasExtension(req, ".png")) {
        res->writeHeader("Content-Type", "image/png");
    } else if(hasExtension(req, ".jpe")) {
        res->writeHeader("Content-Type", "image/jpeg");
    } else if(hasExtension(req, ".jpeg")) {
        res->writeHeader("Content-Type", "image/jpeg");
    } else if(hasExtension(req, ".jpg")) {
        res->writeHeader("Content-Type", "image/jpeg");
    } else if(hasExtension(req, ".gif")) {
        res->writeHeader("Content-Type", "image/gif");
    } else if(hasExtension(req, ".bmp")) {
        res->writeHeader("Content-Type", "image/bmp");
    } else if(hasExtension(req, ".ico")) {
        res->writeHeader("Content-Type", "image/vnd.microsoft.icon");
    } else if(hasExtension(req, ".tiff")) {
        res->writeHeader("Content-Type", "image/tiff");
    } else if(hasExtension(req, ".tif")) {
        res->writeHeader("Content-Type", "image/tiff");
    } else if(hasExtension(req, ".svg")) {
        res->writeHeader("Content-Type", "image/svg+xml");
    } else if(hasExtension(req, ".svgz")) {
        res->writeHeader("Content-Type", "image/svg+xml");
    } else {
        res->writeHeader("Content-Type", "application/text");
    }

    // return res;
}
