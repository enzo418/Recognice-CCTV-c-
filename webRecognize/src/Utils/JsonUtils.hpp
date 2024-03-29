#pragma once

#include <stdexcept>
#include <string>

#include "nlohmann/json.hpp"

namespace Web {

    auto inline GetPathKeys(std::string_view path) {
        // each key represents a node on the configuration, so 50
        // seems reasonably high.
        std::array<std::string, 50> keys;
        int keys_count = 0;

        if (path.empty()) throw std::runtime_error("empty path");

        // split "/"
        while (path.length()) {
            auto pos = path.find("/");
            if (pos == 0) {
                // remove all / at the beggining
                path.remove_prefix(1);
            } else {
                // if didn't found / just add what is left as a key
                pos = pos == std::string::npos ? path.length() : pos;

                if (keys_count >= keys.max_size()) {
                    throw std::runtime_error("too many keys");
                }

                keys[keys_count++] = path.substr(0, pos);
                path.remove_prefix(pos);
            }
        }

        return std::make_tuple(std::move(keys), keys_count);
    }

    /**
     * @brief Generates a json object with properties according to a path.
     * For example for the path zee/foo/bar it generates
     * {
     *  zee: {
     *      foo: {
     *          bar: @value
     *      }
     * }
     *
     * @param path a / separated string of words
     * @param value value to set to the last key
     * @param lastKey will be set to the last key found
     * @return nlohmann::json
     */
    nlohmann::json inline GenerateJsonFromPath(const std::string& path,
                                               const nlohmann::json& value,
                                               std::string* lastKey = nullptr) {
        using json = nlohmann::json;

        auto [keys, keysCount] = GetPathKeys(path);

        json result = json::object();
        json* ref = &result;

        for (int i = 0; i < keysCount; i++) {
            if (i == keysCount - 1) {
                (*ref)[keys[i]] = value;
            } else {
                (*ref)[keys[i]] = json::object();
                ref = &(*ref)[keys[i]];
            }
        }

        if (lastKey) {
            *lastKey = keys[keysCount - 1];
        }

        return result;

        // Another way could be requesting a json as a parameter and use
        // json_pointer to get the reference since it will create the missing
        // keys
        // https://nlohmann.github.io/json/api/basic_json/operator%5B%5D/#notes
        // like so:
        // result[json::json_pointer(path)];
    }

}  // namespace Web
