#pragma once

#include <array>

#include "Configuration.hpp"
#include "YAMLConfiguration.hpp"
#include "yaml-cpp/node/node.h"

namespace Observer::ConfigurationParser {
    // Object is the internal YAML/JSON node representation. It's used to cache
    // configurations, minimizing I/O operations.
    // To be 100% clear it's not a "Configuration" (the struct).
    typedef YAML::Node Object;

    struct ConfigurationFileError : public std::runtime_error {
       public:
        ConfigurationFileError()
            : std::runtime_error("File couldn't be processed") {}

        std::string keyMissing() const { return this->keymissing; }

       private:
        std::string keymissing;
    };

    struct MissingKey : public std::runtime_error {
       public:
        MissingKey(const std::string& pKeymissing)
            : keymissing(std::move(pKeymissing)),
              std::runtime_error("Missing Key '" + pKeymissing + "'") {}

        std::string keyMissing() const { return this->keymissing; }

       private:
        std::string keymissing;
        std::string what_;
    };

    struct WrongType : public std::runtime_error {
       public:
        WrongType(int pLine, int pColumn, int pPosition)
            : mLine(pLine),
              mCol(pColumn),
              mPos(pPosition),
              std::runtime_error("Bad conversion.") {}

        int line() const { return this->mLine; }

        int column() const { return this->mCol; }

        int position() const { return this->mPos; }

       private:
        int mLine;
        int mCol;
        int mPos;
    };

    // yamlcpp
    Configuration ParseYAML(const std::string& filePath);
    void EmmitYAML(const std::string& filePath, const Configuration& cfg);

    Configuration ParseJSON(const std::string& filePath);
    void EmmitJSON(const std::string& filePath, const Configuration& cfg);

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
     * @return true on sucess
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
     * key is the first key to visit on `obj` and iteareate wit the rest of
     * keys until the last one. <value> must be decoded if it comes from
     * an url, but can be anything since we parse it before insertion, see
     * the function above.
     *
     * @return true
     * @return false
     */
    bool TrySetConfigurationFieldValue(Object& obj, std::string_view path);

    /**
     * @brief Reads a configuration object from a string.
     *
     * @param cofiguration
     * @param output
     * @return true on sucess, modifies `output` parameter.
     * @return false on parsin error.
     */
    bool ReadConfigurationObject(const std::string& cofiguration,
                                 Object& output);

    /**
     * @brief Reads a configuration object from a file.
     *
     * @param filePath
     * @param output
     * @return true on sucess, modifies `output` parameter.
     * @return false
     */
    bool ReadConfigurationObjectFromFile(const std::string& filePath,
                                         Object& output);
}  // namespace Observer::ConfigurationParser
