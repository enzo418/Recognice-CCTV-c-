#pragma once

#include <exception>
#include <stdexcept>
#include <string>

namespace Observer::ConfigurationParser {
    struct ConfigurationFileError : public std::runtime_error {
       public:
        ConfigurationFileError()
            : std::runtime_error("File couldn't be processed") {}
    };

    struct MissingKey : public std::runtime_error {
       public:
        MissingKey(const std::string& pKeyMissing)
            : std::runtime_error("Missing Key '" + pKeyMissing + "'"),
              mKeyMissing(std::move(pKeyMissing)) {}

        std::string keyMissing() const { return this->mKeyMissing; }

       private:
        std::string mKeyMissing;
        std::string what_;
    };

    struct WrongType : public std::runtime_error {
       public:
        WrongType(int pLine, int pColumn, int pPosition)
            : std::runtime_error("Bad conversion."),
              mLine(pLine),
              mCol(pColumn),
              mPos(pPosition) {}

        WrongType(std::string pGot, std::string pExpected, std::string pValue)
            : std::runtime_error("Bad conversion."),
              mGot(std::move(pGot)),
              mExpected(std::move(pExpected)),
              mValue(std::move(pValue)) {}

        int line() const { return this->mLine; }

        int column() const { return this->mCol; }

        int position() const { return this->mPos; }

        std::string got() const { return this->mGot; }

        std::string expected() const { return this->mExpected; }

        std::string value() const { return this->mValue; }

       private:
        int mLine;
        int mCol;
        int mPos;
        std::string mGot;
        std::string mExpected;
        std::string mValue;
    };

}  // namespace Observer::ConfigurationParser