#pragma once

#include <chrono>
#include <deque>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <thread>

#include "../SocketData.hpp"
#include "Pattern/VideoBufferSubscriberPublisher.hpp"
#include "Serialization/JsonSerialization.hpp"
#include "Streaming/WebsocketService.hpp"
#include "nlohmann/json.hpp"
#include "observer/BlockingFIFO.hpp"
#include "observer/Functionality.hpp"
#include "observer/Log/log.hpp"
#include "observer/Semaphore.hpp"

namespace Web {

    template <bool SSL>
    class WebsocketVideoBufferController final
        : public Observer::Functionality,
          public Streaming::WebsocketService<SSL, VideoBufferSocketData>,
          public VideoBufferSubscriber {
        typedef uWS::WebSocket<SSL, true, VideoBufferSocketData>* Client;

       public:
        void update(std::string bufferId, int EventType, std::string data);

        void SendInitialBuffer(Client client, const std::string& buffer);

       protected:
        struct Event {
            const std::string bufferID;
            const int eventType;
            const std::string data;
        };

       private:
        void InternalStart();

       private:
        Semaphore smpQueue;
        Observer::BlockingFIFO<Event> events;
    };

    std::string inline EventTypeToString(int type) {
        switch (type) {
            case BufferEventType::CANCELED:
                return "canceled";
            case BufferEventType::BUFFER_READY:
                return "ready";
            case BufferEventType::DETECTION_DONE:
                return "detection_finished";
            case BufferEventType::UPDATED:
                return "updated";
            default:
                throw std::runtime_error("uknown event type");
        }
    }

    template <bool SSL>
    void WebsocketVideoBufferController<SSL>::InternalStart() {
        while (this->running) {
            if (smpQueue.acquire_timeout<250>()) {
                OBSERVER_ASSERT(events.size() > 0, "semaphore logic error");

                while (events.size() > 0) {
                    const Event event = events.pop_front();
                    const std::string payload =
                        EventTypeToString(event.eventType) + ":" + event.data;

                    this->SendToSomeClients(
                        payload.c_str(), payload.size(),
                        [&buffID = event.bufferID](void* _client) {
                            auto client = (Client)_client;
                            return client->getUserData()->bufferID == buffID;
                        });
                }
            }
        }
    }

    template <bool SSL>
    void WebsocketVideoBufferController<SSL>::update(std::string bufferId,
                                                     int eventType,
                                                     std::string data) {
        events.push_back(
            Event {.bufferID = bufferId, .eventType = eventType, .data = data});
        smpQueue.release();
    }

    template <bool SSL>
    void WebsocketVideoBufferController<SSL>::SendInitialBuffer(
        Client client, const std::string& buffer) {
        const std::string message =
            EventTypeToString(BufferEventType::UPDATED) + ":" + buffer;
        client->send(std::string_view(message.c_str(), message.size()),
                     uWS::OpCode::BINARY, true);
    }
}  // namespace Web