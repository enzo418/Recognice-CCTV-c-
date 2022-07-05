#ifndef FILE_STREAMER
#define FILE_STREAMER

#include <filesystem>
#include <iostream>

#include "observer/Log/log.hpp"
#include "../../uWebSockets/src/App.h"
#include "../server_utils.hpp"
#include "FileExtension.hpp"
#include "FileReader.hpp"
#include "Range.hpp"
#include "StaticFilesHandler.hpp"

// Min buffer: 32kb = 32.768 bytes
const size_t ReadWriteBufferSize = 32768;

// Max buffer size: 2MB = 2e6 bytes
const long MaxBufferSize = 2 * 1000 * 1000;

class FileStreamer {
   public:
    template <bool SSL>
    static FileStreamer& Init(const std::string& root) {
        return GetInstanceImpl<SSL>(root);
    }

    static FileStreamer& GetInstance();

    template <bool SSL>
    bool streamRangedFile(uWS::HttpResponse<SSL>* res, const std::string& url,
                          FileReader* fileReader,
                          const std::string& rangeHeader) {
        /**
         * This function stream a ranged file to the client.
         * First it tells the client that the content is a Partial content
         * that accepts a range from 0 to the file size.
         *
         * For a mediafile, e.g. a mp4 video file the
         * Content type is text/html and not a video/audio since
         * in this response we don't send actually any data of the file.
         *
         * Then we start sending to the user the actual file data.
         **/
        const long fz = fileReader->getFileSize();

        std::vector<Range> ranges;

        if (!parseRangeHeader(rangeHeader, fz, ranges)) {
            std::cout << "invalid header range \"" << rangeHeader << "\"\n";
            return false;
        }

        // std::cout << "Ranges: " << rangeHeader << " | File size: " << fz <<
        // std::endl; for(auto&& range : ranges) {
        //     std::cout << "\tstart: " << range.start << " -> end: " <<
        //     range.end << std::endl;
        // }

        std::string rangesStringOut = getRangeStringForResponse(ranges[0], fz);

        // Check file health before reading it
        if (!fileReader->good()) {
            // Maybe it reached the end before?
            fileReader->clear();  // clear eof or bad state
            // Check it again
            if (!fileReader->good()) {
                // Maybe it was moved, deleted or changed. Try to re open it
                fileReader->reload();
                if (!fileReader->good()) {
                    // Definitely file isn't longer here. Should send 404 code!
                    std::cout << "File isn't good\n";
                }
                return false;
            }
        }

        // TODO: Use some real world streaming algorithm like
        // https://en.wikipedia.org/wiki/Streaming_media#Bandwidth

        // use the smallest buffer size on the first request
        // this is for example, if a uer request a video we respond with a small
        // part of the data to let the browser/program know that the media is
        // available and can be played. After the users hits play we start to
        // use a bigger buffer
        auto contentLength =
            std::min(fz - ranges[0].start, (long)ReadWriteBufferSize);

        // always write status first
        res->writeStatus("206 Partial Content")
            ->writeHeader("Content-Range",
                          std::string_view(rangesStringOut.data(),
                                           rangesStringOut.length()))
            ->writeHeader("Content-Length", contentLength)
            ->writeHeader("Connection", "keep-alive")
            ->writeHeader("Accept-Ranges", "bytes")
            ->writeHeader("Content-Type",
                          getContentType(fileReader->getFileName()))
            ->writeHeader("Last-Modified",
                          "Thu, 17 Jun 2021 20:50:11 GMT")  // TODO: set actual
                                                            // modified time
            ->writeHeader("Access-Control-Allow-Origin", "*")
            ->tryEndRaw(std::string_view(nullptr, 0), 0);

        res->onAborted(
            [url]() { std::cout << "[" << url << "] ABORTED!" << std::endl; });

        std::string ran;
        std::string buf;
        buf.resize(contentLength);

        for (auto& range : ranges) {
            fileReader->seek(range.start, fileReader->beg());

            auto bytesLeft = range.length();

            while (bytesLeft) {
                auto of = std::min(buf.length(), bytesLeft);

                fileReader->read(buf.data(), of);
                if (of <= 0) {
                    const static std::string unexpectedEof("Unexpected EOF");
                    std::cout << "Error reading file: "
                              << (of == 0 ? unexpectedEof : "getLastError")
                              << std::endl;
                    // We can't send an error document as we've sent the header.
                    return false;
                }

                if (!fileReader->good()) {
                    OBSERVER_WARN(
                        "Ranged file stream - file isn't good! file: {}", url);
                }

                bytesLeft -= of;

                if (!res->tryEndRaw(buf, of).first) {
                    break;
                }
            }
        }

        return true;
    }

    template <bool SSL>
    bool streamChunkedFile(uWS::HttpResponse<SSL>* res, const std::string& url,
                           FileReader* fileReader) {
        /**
         * When the server request a file we answer with
         *  -   Status: 200 OK
         *  -   Transfer-Encoding: chunked
         * in the first chunk
         *
         * Then (without closing the connection [end]) we send the chunk as
         * specified here:
         *https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Transfer-Encoding#directives
         *
         * When we reach the end of file then we send an chunk with 0 size,
         * if sucess then the client should have responded with an FIN flag and
         *we of course try to end as the user said.
         *
         * Tranfer-Enconding chunk doesn't allow to seek for a specific range on
         *the file, so is not a good idea for media files.
         **/

        long fz = fileReader->getFileSize();

        res->onAborted(
            [url]() { std::cout << "[" << url << "] ABORTED!" << std::endl; });

        // Check file health before reading it
        if (!fileReader->good()) {
            // Maybe it reached the end before?
            fileReader->clear();  // clear eof or bad state
            // Check it again
            if (!fileReader->good()) {
                // Maybe it was moved, deleted or changed. Try to re open it
                fileReader->reload();
                if (!fileReader->good()) {
                    // Definitely file isn't longer here. Should send 404 code!
                    std::cout << "File isn't good\n";
                }
                return false;
            }
        }

        std::string ran;
        std::string buf;
        buf.resize(ReadWriteBufferSize);

        fileReader->seek(0, fileReader->beg());
        long total = 0;

        auto bytesLeft = fz;
        while (bytesLeft) {
            auto of = std::min((long)buf.length(), bytesLeft);
            total += of;

            // ran = "bytes " + std::to_string(total) + "/" +
            // std::to_string(fz); std::cout << "Reading " << of << " bytes from
            // file." << " Left: " << bytesLeft <<  " Ran: " << ran <<
            // std::endl;
            fileReader->read(buf.data(), of);
            if (of <= 0) {
                const static std::string unexpectedEof("Unexpected EOF");
                std::cout << "Error reading file: "
                          << (of == 0 ? unexpectedEof : "getLastError")
                          << std::endl;
                // We can't send an error document as we've sent the header.
                return false;
            }

            bytesLeft -= of;
            if (!res->write(std::string_view(buf.data(), of))) {
                // This can be expected because
                // client may only want the first part of the file.
                // Do not return
                // std::cout << "Client ended range" << std::endl;
            }
        }

        // Send 0 size chunk to tell the client that we don't have
        // more content
        if (!res->writeOrZero(std::string_view(nullptr, 0))) {
            // res->close();
            // res->end();
            std::cout << "Error sending 0 size chunk\n";
            return false;
        }

        // everything was ok, end it.
        res->tryEnd(std::string_view(nullptr, 0), 0);
        return true;
    }

    template <bool SSL>
    bool streamFile(uWS::HttpResponse<SSL>* res, std::string url,
                    std::string rangeHeader, std::string directory = "") {
        if (directory.empty()) directory = root;
        if (url[0] == '/') url = url.substr(1, url.length() - 1);

        // /path/to/directory / url without initial '/'
        const std::string path =
            (std::filesystem::path(directory) / url).lexically_normal();

        this->filesHandlerMtx.lock();

        // yes, in each request we check if the file exists
        // if it exists and doesn't have a handler then
        if (!staticFiles->fileExists(path)) {
            staticFiles->removeHandlerIfExists(path);

            OBSERVER_WARN("{0} Did not find file: {1}", HTTP_404_NOT_FOUND,
                          path);

            res->writeStatus(HTTP_404_NOT_FOUND);
            res->end();

            this->filesHandlerMtx.unlock();
            return false;
        } else if (!staticFiles->fileHandlerExists(path)) {
            staticFiles->addFileHandler(path);
        }

        FileReader* reader = staticFiles->getFileHandler(path);

        this->filesHandlerMtx.unlock();

        // request is not ranged, send it chunked
        if (rangeHeader.empty()) {
            return streamChunkedFile(res, url, reader);
        } else {
            // there is a range
            return streamRangedFile(res, url, reader, rangeHeader);
        }
    }

   private:
    StaticFilesHandler* staticFiles;
    std::string root;

    std::mutex filesHandlerMtx;

    static FileStreamer* instance;

   private:
    FileStreamer(std::string root);

    FileStreamer(FileStreamer const&);
    void operator=(FileStreamer const&);

    template <bool SSL>
    static FileStreamer& GetInstanceImpl(const std::string& root) {
        OBSERVER_ASSERT(instance == nullptr,
                        "Multiple initializations of filestreamer");

        instance = new FileStreamer(root);

        return *instance;
    }
};

#endif /* FILE_STREAMER */