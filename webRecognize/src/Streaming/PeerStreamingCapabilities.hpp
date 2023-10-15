#pragma once

namespace Web::Streaming {
    struct PeerStreamingCapabilities {
        bool supportsJpgCacheBusting;
        bool supportsMJPEGStream;
        bool supportsH264Stream;
        bool supportsWebRTC;
    };
}  // namespace Web::Streaming