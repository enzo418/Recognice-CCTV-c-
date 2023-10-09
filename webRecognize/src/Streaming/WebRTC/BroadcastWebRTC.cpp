#include "BroadcastWebRTC.hpp"

#include "observer/Log/log.hpp"

namespace Web::Streaming::WebRTC {
    void MultiBroadcastWebRTC::OnChannelNotFound(const std::string& sourceId) {
        std::unique_ptr<broadrtc::BroadcastSource> source;

        if (sourceId == OBSERVER_SRC_TAG) {
            OBSERVER_ASSERT(recognizeCtx != nullptr,
                            "RecognizeContext is null");

            if (recognizeCtx->observer == nullptr) {
                throw std::runtime_error("observer not yet initialized");
            }

            if (!recognizeCtx->running) {
                OBSERVER_TRACE("Not running but will subscribe anyway");
            }

            // create source and subscribe to frames
            ObserverSource* observerSource = new ObserverSource();
            recognizeCtx->observer->SubscribeToFrames(observerSource);

            // let unique_ptr handle it
            source.reset(observerSource);
        } else {
            source = std::make_unique<GenericSource>(sourceId);
        }

        rtc::scoped_refptr<broadrtc::BroadcastTrackSource> trackSource =
            broadrtc::BroadcastTrackSource::Create(std::move(source));

        m_mutexSourcesMap.lock();
        m_sources.insert({sourceId, std::move(trackSource)});
        m_mutexSourcesMap.unlock();
    }

    void MultiBroadcastWebRTC::OnObserverStopped() {
        m_sources.erase(OBSERVER_SRC_TAG);
    }
}  // namespace Web::Streaming::WebRTC