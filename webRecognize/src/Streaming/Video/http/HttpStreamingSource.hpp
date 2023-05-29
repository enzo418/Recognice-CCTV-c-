#pragma once

#include "observer/Domain/BufferedSource.hpp"
#include "observer/Functionality.hpp"
// namespace Web::Streaming::Video::Http {
//     template <typename SSL>
//     class HttpStreamingSource : public Observer::Functionality {
//        public:
//         HttpStreamingSource(const std::string& source);
//         ~HttpStreamingSource();

//        protected:
//         void InternalStart() override;
//         void PostStop() override;

//        private:
//         Observer::BufferedSource source;
//         std::string cameraUri;
//     };

//     template <typename SSL>
//     HttpStreamingSource<SSL>::HttpStreamingSource(const std::string& source)
//     {}
// }  // namespace Web::Streaming::Video::Http