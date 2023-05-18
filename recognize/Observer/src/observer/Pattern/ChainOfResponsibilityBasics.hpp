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
template <typename T, typename... R>
class Handler {
   public:
    virtual Handler* SetNext(Handler* handler) = 0;
    virtual T Handle(R... args) = 0;

    virtual ~Handler() = default;
};

/**
 * The default chaining behavior will be implemented inside a base handler
 * class.
 * @tparam T
 * @tparam R
 */
template <typename T, typename... Args>
class AbstractHandler : public Handler<T, Args...> {
   protected:
    Handler<T, Args...>* nextHandler;

   public:
    AbstractHandler() : nextHandler(nullptr) {}

    Handler<T, Args...>* SetNext(Handler<T, Args...>* handler) override {
        this->nextHandler = handler;

        return handler;
    }

    T Handle(Args... args) override {
        if (this->nextHandler) {
            return this->nextHandler->Handle(args...);
        }

        if constexpr (!std::is_void<T>::value) {
            return {};
        }
    }
};