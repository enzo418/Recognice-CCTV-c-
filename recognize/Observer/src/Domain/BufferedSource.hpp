#pragma once

#include "../Functionality.hpp"
#include "../ImageTransformation.hpp"
#include "../Log/log.hpp"
#include "../SimpleBlockingQueue.hpp"
#include "VideoSource.hpp"

namespace Observer {

    /**
     * @brief This class provides a clean connection to a source, as it will
     * read all frames to create a buffer, from which frames can be requested
     * without worrying about timing.
     *
     * Use only to a single consumer.
     *
     * @tparam TFrame
     */
    template <typename TFrame>
    class BufferedSource : private Functionality {
       public:
        BufferedSource() = default;

        /**
         * @brief Tries to open a connection with given source.
         * If we could open it a thread will be created and will return true,
         * else we just return false.
         *
         * @param sourceUri
         * @return true
         * @return false
         */
        bool TryOpen(const std::string& sourceUri);

        /**
         * @brief Close the connection to the source.
         * Also joins the thread.
         */
        void Close();

        /**
         * @brief Get a frame. Before calling this function you should check
         * wether a frame is avabilable or not.
         *
         * Data race: In case of multiple consumers while it may say a frame is
         * available another consumer already consumed it, and when you try to
         * get it will crash.
         * @return TFrame
         */
        TFrame GetFrame();

        bool IsFrameAvailable();

        bool IsOk();

        double GetFPS();

       private:
        bool TryOpenConnection(const std::string& sourceUri);

       protected:
        void InternalStart() override;

       protected:
        SimpleBlockingQueue<TFrame> queue;
        VideoSource<TFrame> source;
        std::string sourceUri;
    };

    template <typename TFrame>
    bool BufferedSource<TFrame>::TryOpen(const std::string& sourceUri) {
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

    template <typename TFrame>
    void BufferedSource<TFrame>::Close() {
        this->Stop();

        if (source.isOpened()) {
            source.Close();
        }
    }

    template <typename TFrame>
    double BufferedSource<TFrame>::GetFPS() {
        return source.GetFPS();
    }

    template <typename TFrame>
    void BufferedSource<TFrame>::InternalStart() {
        if (!this->IsOk()) {
            OBSERVER_WARN(
                "Connection to source couldn't be stablished. URI: {}",
                sourceUri);

            this->Stop();
            return;
        }

        // double fps = source.GetFPS();
        // if (fps == 0) {
        // }

        // double expectedMsBetweenFrames = source.GetFPS();
        TFrame frame;

        while (running) {
            // do stuff
            if (source.GetNextFrame(frame)) {
                if (!ImageTransformation<TFrame>::GetSize(frame).empty()) {
                    queue.push(ImageTransformation<TFrame>::CloneImage(frame));
                }
            } else {
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

    template <typename TFrame>
    bool BufferedSource<TFrame>::TryOpenConnection(
        const std::string& sourceUri) {
        source.Open(sourceUri);

        return source.isOpened();
    }

    template <typename TFrame>
    TFrame BufferedSource<TFrame>::GetFrame() {
        // Don't worry it's (supposedly) a Single consumer buffer
        return queue.pop();
    }

    template <typename TFrame>
    bool BufferedSource<TFrame>::IsFrameAvailable() {
        return queue.size() > 0;
    }

    template <typename TFrame>
    bool BufferedSource<TFrame>::IsOk() {
        return running && source.isOpened();
    }

}  // namespace Observer