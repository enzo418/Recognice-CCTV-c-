#pragma once

#include <iostream>
#include <vector>

/**
 * This file implements the basics of the
 * chain of responsibility pattern to use
 * it as a validator:
 *      There will be multiple handlers, but the default will return true,
 *      meaning that if none could determine that it was invalid, the
 *      result is valid.
*/

 /**
  * The Handler interface declares a method for building the chain of handlers.
  * It also declares a method for executing a request.
  * @tparam T
  * @tparam R
  */
template <typename T, typename R>
class Handler {
public:
    virtual Handler *SetNext(Handler *handler) = 0;
    virtual T Handle(R request, T& result) = 0;

    virtual ~Handler() = 0;
};

 /**
  * The default chaining behavior will be implemented inside a base handler class.
  * @tparam T
  * @tparam R
  */
template <typename T, typename R>
class AbstractHandler : public Handler<T, R> {
private:
    Handler<T, R> *nextHandler;

public:
    AbstractHandler() : nextHandler(nullptr) {    }

    Handler<T, R> *SetNext(Handler<T, R> *handler) override {
        this->nextHandler = handler;

        return handler;
    }

    T Handle(R request, T& result) override {
        if (this->nextHandler) {
            return this->nextHandler->Handle(request, result);
        }

        return result;
    }
};

struct ValidationResult {
public:
    ValidationResult() : valid(false) {} // for use as default inside template

    explicit ValidationResult(bool pResult) : valid(pResult) {}

    ValidationResult(bool pResult, std::vector<std::string>& pMessages) :
            valid(pResult), messages(pMessages)
    { }

    bool valid = false;
    std::vector<std::string> messages{};
};