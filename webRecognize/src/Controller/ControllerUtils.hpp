#pragma once

#include "server_utils.hpp"
#include "uWebSockets/App.h"

#define READ_JSON_BODY(res, req, func)                            \
    res->onAborted([]() {});                                      \
    std::string buffer;                                           \
    auto ct = std::string(req->getHeader("content-type"));        \
    res->onData([this, res, req, ct, buffer = std::move(buffer)]( \
                    std::string_view data, bool last) mutable {   \
        buffer.append(data.data(), data.length());                \
        if (last) {                                               \
            if (ct != "application/json") {                       \
                res->writeStatus(HTTP_400_BAD_REQUEST)            \
                    ->end("Expected a json body");                \
                return;                                           \
            }                                                     \
            nlohmann::json parsed;                                \
            try {                                                 \
                parsed = nlohmann::json::parse(buffer);           \
            } catch (const std::exception& e) {                   \
                res->writeStatus(HTTP_400_BAD_REQUEST)            \
                    ->end("Body is not a valid json");            \
                return;                                           \
            }                                                     \
                                                                  \
            func(res, req, parsed);                               \
        }                                                         \
    });

#define READ_JSON_BODY_TYPED(res, req, func, type)                \
    res->onAborted([]() {});                                      \
    std::string buffer;                                           \
    auto ct = std::string(req->getHeader("content-type"));        \
    res->onData([this, res, req, ct, buffer = std::move(buffer)]( \
                    std::string_view data, bool last) mutable {   \
        buffer.append(data.data(), data.length());                \
        if (last) {                                               \
            if (ct != "application/json") {                       \
                res->writeStatus(HTTP_400_BAD_REQUEST)            \
                    ->end("Expected a json body");                \
                return;                                           \
            }                                                     \
            nlohmann::json parsed;                                \
            try {                                                 \
                parsed = nlohmann::json::parse(buffer);           \
            } catch (const std::exception& e) {                   \
                res->writeStatus(HTTP_400_BAD_REQUEST)            \
                    ->end("Body is not a valid json");            \
                return;                                           \
            }                                                     \
                                                                  \
            type parsedTyped;                                     \
                                                                  \
            try {                                                 \
                parsedTyped = parsed;                             \
            } catch (const std::exception& e) {                   \
                res->writeStatus(HTTP_400_BAD_REQUEST)            \
                    ->end("Unexpected json type");                \
                return;                                           \
            }                                                     \
                                                                  \
            func(res, req, parsedTyped);                          \
        }                                                         \
    });