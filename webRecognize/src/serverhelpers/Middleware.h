#pragma once

#include <string>
#include <uWebSockets/App.h>

/* Middleware to fill out content-type */
inline bool hasExt(std::string_view file, std::string_view ext) {
    if (ext.size() > file.size()) {
        return false;
    }
    return std::equal(ext.rbegin(), ext.rend(), file.rbegin());
}

/* This should be a filter / middleware like app.use(handler) */
template <bool SSL>
uWS::HttpResponse<SSL> *serveFile(uWS::HttpResponse<SSL> *res, uWS::HttpRequest *req, const bool isRoot = false) {
    if (hasExt(req->getUrl(), ".svg")) {
        res->writeHeader("Content-Type", "image/svg+xml");
    } else if(hasExt(req->getUrl(), ".htm")) {
        res->writeHeader("Content-Type", "text/html");
    } else if(isRoot || hasExt(req->getUrl(), ".html")) {
        res->writeHeader("Content-Type", "text/html");
    } else if(hasExt(req->getUrl(), ".php")) {
        res->writeHeader("Content-Type", "text/html");
    } else if(hasExt(req->getUrl(), ".css")) {
        res->writeHeader("Content-Type", "text/css");
    } else if(hasExt(req->getUrl(), ".txt")) {
        res->writeHeader("Content-Type", "text/plain");
    } else if(hasExt(req->getUrl(), ".js")) { 
        res->writeHeader("Content-Type", "application/javascript");
    } else if(hasExt(req->getUrl(), ".json")) {
        res->writeHeader("Content-Type", "application/json");
    } else if(hasExt(req->getUrl(), ".xml")) {
        res->writeHeader("Content-Type", "application/xml");
    } else if(hasExt(req->getUrl(), ".swf")) {
        res->writeHeader("Content-Type", "application/x-shockwave-flash");
    } else if(hasExt(req->getUrl(), ".flv")) {
        res->writeHeader("Content-Type", "video/x-flv");
    } else if(hasExt(req->getUrl(), ".png")) {
        res->writeHeader("Content-Type", "image/png");
    } else if(hasExt(req->getUrl(), ".jpe")) {
        res->writeHeader("Content-Type", "image/jpeg");
    } else if(hasExt(req->getUrl(), ".jpeg")) {
        res->writeHeader("Content-Type", "image/jpeg");
    } else if(hasExt(req->getUrl(), ".jpg")) {
        res->writeHeader("Content-Type", "image/jpeg");
    } else if(hasExt(req->getUrl(), ".gif")) {
        res->writeHeader("Content-Type", "image/gif");
    } else if(hasExt(req->getUrl(), ".bmp")) {
        res->writeHeader("Content-Type", "image/bmp");
    } else if(hasExt(req->getUrl(), ".ico")) {
        res->writeHeader("Content-Type", "image/vnd.microsoft.icon");
    } else if(hasExt(req->getUrl(), ".tiff")) {
        res->writeHeader("Content-Type", "image/tiff");
    } else if(hasExt(req->getUrl(), ".tif")) {
        res->writeHeader("Content-Type", "image/tiff");
    } else if(hasExt(req->getUrl(), ".svg")) {
        res->writeHeader("Content-Type", "image/svg+xml");
    } else if(hasExt(req->getUrl(), ".svgz")) {
        res->writeHeader("Content-Type", "image/svg+xml");
    } else {
        res->writeHeader("Content-Type", "application/text");
    }

    return res;
}
