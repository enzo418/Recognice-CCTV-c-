#pragma once

#include <algorithm>
#include <iostream>
#include <set>
#include <vector>

/**
 * This unit provides a observe pattern implementation.
 * Subscribers are notified blocking the publisher thread, that way it's able to
 * modify the value if it wants or copy it and use any asynchronous technique to
 * process it.
 */

// Inteface Subscriber
template <typename... Ctx>
class ISubscriber {
   public:
    virtual void update(Ctx...) = 0;
};

namespace Observer {
    enum Priority : uint8_t { HIGH = 0, MEDIUM, LOW };
};

// Publisher base class
template <typename... Ctx>
class Publisher {
   public:
    typedef ISubscriber<Ctx...> Subscriber;

   private:
    typedef Subscriber* SubscriberPtr;

    struct PrioritySubscriber {
        Observer::Priority priority;
        SubscriberPtr subscriber;
    };

    struct PrioritySubscriberCmp {
        bool operator()(const PrioritySubscriber& lhs,
                        const PrioritySubscriber& rhs) const {
            return lhs.priority < rhs.priority;
        }
    };

    typedef std::set<PrioritySubscriber, PrioritySubscriberCmp> SubscriberList;

   public:
    /**
     * @brief Observe this publisher.
     *
     * @param subscriber
     * @param priority Priority of the subscriber. Higher priority will be
     * called first. If two subscribers have the same priority, the first that
     * subscribed will be called first.
     */
    void subscribe(SubscriberPtr subscriber,
                   Observer::Priority priority = Observer::Priority::LOW) {
        subscribers.insert(PrioritySubscriber {priority, subscriber});
    }

    void unsubscribe(SubscriberPtr subscriber) {
        auto find = std::find(subscribers.begin(), subscribers.end(),
                              [&subscriber](PrioritySubscriber& prioritySub) {
                                  return prioritySub.subscriber == subscriber;
                              });
        if (find != this->subscribers.end()) {
            this->subscribers.erase(find);
        }
    }

    void notifySubscribers(Ctx... args) {
        for (auto& obs : subscribers) {
            obs.subscriber->update(args...);
        }
    }

   private:
    SubscriberList subscribers;
};
