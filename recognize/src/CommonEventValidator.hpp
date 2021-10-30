#pragma once

#include "Event.hpp"

#include <iostream>
#include <vector>

namespace Observer {
    struct ValidationResult {
    public:
        ValidationResult() : valid(false) {} // for use as default inside template

        explicit ValidationResult(bool pResult) : valid(pResult) {}

        ValidationResult(bool pResult, std::vector <std::string> &pMessages) :
                valid(pResult), messages(pMessages) {}

        bool valid = false;
        std::vector <std::string> messages{};
    };
}