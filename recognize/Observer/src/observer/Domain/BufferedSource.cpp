#include "BufferedSource.hpp"

constexpr double MAX_FRAMES_IN_QUEUE = 100;

namespace Observer {
    bool BufferedSource::TryOpen(const std::string& sourceUri) {
        // Close all
        if (running) {
            this->Close();
            OBSERVER_WARN("Calling open on buffer while it's running!");
        }

        bool opened = this->TryOpenConnection(sourceUri);

        if (opened) {
            Functionality::Start();
        }

        return opened;
    }

    void BufferedSource::Close() {
        this->Stop();

        if (source.isOpened()) {
            source.Close();
        }
    }

    double BufferedSource::GetFPS() { return source.GetFPS(); }

    void BufferedSource::InternalStart() {
        if (!this->IsOk()) {
            OBSERVER_WARN(
                "Connection to source couldn't be stabilized. URI: {}",
                sourceUri);

            this->Stop();
            return;
        }

        Frame frame;

        while (running) {
            // do stuff
            if (source.GetNextFrame(frame)) {
                if (queue.size() >= MAX_FRAMES_IN_QUEUE) queue.pop();

                if (!frame.IsEmpty()) {
                    queue.push(frame.Clone());
                }
            } else if (!source.isOpened()) {
                // something went wrong.
                OBSERVER_WARN("Connection to source was lost. URI: {}",
                              sourceUri);
                break;
            }
        }

        // if something went wrong
        if (running) {
            this->Stop();
        }
    }

    bool BufferedSource::TryOpenConnection(const std::string& sourceUri) {
        source.Open(sourceUri);

        return source.isOpened();
    }

    Frame BufferedSource::GetFrame() { return queue.pop(); }

    bool BufferedSource::IsFrameAvailable() { return queue.size() > 0; }

    bool BufferedSource::IsOk() { return running && source.isOpened(); }

    int BufferedSource::BufferedAmount() { return queue.size(); }

    Size BufferedSource::GetInputResolution() { return this->source.GetSize(); }

}  // namespace Observer