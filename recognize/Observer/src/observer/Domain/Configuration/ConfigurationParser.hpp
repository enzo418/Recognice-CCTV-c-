#pragma once

#include <array>

#include "Configuration.hpp"
#include "NLHJSONConfiguration.hpp"
#include "nlohmann/json.hpp"
#include "observer/RecoJson.hpp"

namespace Observer::ConfigurationParser {
    // Object is the internal YAML/JSON node representation. It's used to cache
    // configurations, minimizing I/O operations.
    typedef json Object;

    std::string NodeAsJson(nlohmann::json& obj);

    void ConfigurationToJsonFile(const std::string& filePath,
                                 const Configuration& cfg);

    Configuration ConfigurationFromJsonFile(const std::string& filePath);

    std::string GetConfigurationAsJSON(const Configuration& cfg);

    /**
     * @brief Tries to set a value to a node located at `keys`.
     * `value` is is parsed before insertion, that means if you
     * pass a sequence as [1,2,3] it actually creates multiple
     * nodes as child of the node at `keys`.
     *
     * @param obj Node to iterate with the keys
     * @param keys key in `obj`. Each key is a string, but the content is not
     * limited to letters, that means "0" can be used to access the first
     * element of a sequence and "name" to access the name value of `obj`.
     * @param keysCount
     * @param value
     * @return true on success
     * @return false on key not found
     */
    bool TrySetNodeValue(Object& obj, std::string* keys, int keysCount,
                         const std::string& value);

    /**
     * @brief Same as above, but makes easier the call using a `path`
     * as the location to where put the value.
     *
     * Use of string instead of string_view is due to the yaml-cpp not
     * supporting it to access nodes by key.
     *
     * @param obj Node
     * @param path string with the syntax: <key_1>/<key_2>/.../?to=<value> where
     * key is the first key to visit on `obj` and iterate wit the rest of
     * keys until the last one. <value> must be decoded if it comes from
     * an url, but can be anything since we parse it before insertion, see
     * the function above.
     *
     * @return true
     * @return false
     */
    bool TrySetConfigurationFieldValue(Object& obj, std::string_view path);

    /**
     * @brief same as TrySetNodeValue but return a Node with the value if
     * found, else returns json null
     * @param obj
     * @param keys
     * @param keysCount
     * @param value
     * @return json
     */
    json TryGetNodeValue(const Object& obj, std::string* keys, int keysCount);

    /**
     * @brief Follows the same rules as the functions above.
     *
     * @param obj Node
     * @param path to get key_n pass: <key_1>/<key_2>/.../<key_n>, see
     * TrySetConfigurationFieldValue
     * @param output
     * @return json
     */
    json TryGetConfigurationFieldValue(Object& obj, std::string_view path);

}  // namespace Observer::ConfigurationParser
