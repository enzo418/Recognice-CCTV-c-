#pragma once

#include "observer/Functionality.hpp"
#include "observer/Implementation.hpp"
#include "observer/Log/log.hpp"
#include "observer/SimpleBlockingQueue.hpp"

namespace Observer {
    /**
     * @brief This class provides a clean connection to a source, as it will
     * read all frames to create a buffer, from which frames can be requested
     * without worrying about timing.
     *
     * BE SURE TO CALL START after opening the camera and BEFORE CONSUMING the
     * frame
     *
     * Use only to a single consumer.
     *
     * @tparam TFrame
     */
    class BufferedSource : public Functionality {
       public:
        BufferedSource() = default;

        /**
         * @brief Tries to open a connection with given source.
         * If we could open it a thread will be created and will return true,
         * else we just return false.
         * Doesn't start getting frames, call Start for that.
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
         * wether a frame is available or not.
         *
         * Data race: In case of multiple consumers while it may say a frame is
         * available another consumer already consumed it, and when you try to
         * get it will crash.
         * @return TFrame
         */
        Frame GetFrame();

        bool IsFrameAvailable();

        bool IsOk();

        double GetFPS();

        int BufferedAmount();

        Size GetInputResolution();

       private:
        bool TryOpenConnection(const std::string& sourceUri);

        bool CheckCanStart() override;

       protected:
        void InternalStart() override;

       protected:
        SimpleBlockingQueue<Frame> queue;
        VideoSource source;
        std::string sourceUri;
    };
}  // namespace Observer