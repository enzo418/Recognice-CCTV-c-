#pragma once

#include "Configuration.hpp"
#include "YAMLConfiguration.hpp"

namespace Observer::ConfigurationParser {
    struct MissingKey : public std::exception {
       public:
        MissingKey(const std::string& pKeymissing)
            : keymissing(std::move(pKeymissing)) {}

        const char* what() const throw() {
            return ("Missing Key '" + this->keymissing + "'").c_str();
        }

        std::string keyMissing() const { return this->keymissing; }

       private:
        std::string keymissing;
    };

    struct WrongType : public std::exception {
       public:
        WrongType(int pLine, int pColumn, int pPosition)
            : mLine(pLine), mCol(pColumn), mPos(pPosition) {}

        const char* what() const throw() { return "Bad conversion."; }

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
}  // namespace Observer::ConfigurationParser
