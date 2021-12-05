#pragma once

#include <algorithm>
#include <iostream>
#include <vector>

// Inteface Subscriber
template <typename... Ctx>
class ISubscriber {
   public:
    virtual void update(Ctx...) = 0;
};

// Publisher base class
template <typename... Ctx>
class Publisher {
    typedef std::vector<ISubscriber<Ctx...>*> SubscriberList;

   public:
    void subscribe(ISubscriber<Ctx...>* subscriber) {
        subscribers.push_back(subscriber);
    }

    void unsubscribe(ISubscriber<Ctx...>* subscriber) {
        auto find =
            std::find(subscribers.begin(), subscribers.end(), subscriber);
        if (find != this->subscribers.end()) {
            this->subscribers.erase(find);
        }
    }

    void notifySubscribers(Ctx... args) {
        for (auto& obs : subscribers) {
            obs->update(args...);
        }
    }

   private:
    SubscriberList subscribers;
};
