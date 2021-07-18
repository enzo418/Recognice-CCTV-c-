#pragma once
#include <filesystem>
#include "FileExtension.h"
#include "Range.h"
#include <iostream>
#include "../server_utils.hpp"

// Max buffer size: 16 kb = 16.384 bytes
constexpr size_t ReadWriteBufferSize = 16 * 1024;

struct FileStreamer {

    StaticFilesHandler* staticFiles;
    std::string root;

    FileStreamer(std::string root) : root(root) {
        staticFiles = new StaticFilesHandler(root);
    }

    template <bool SSL>
    bool streamRangedFile(uWS::HttpResponse<SSL> *res, const std::string& url, FileReader *fileReader, const std::string& rangeHeader) {
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
        
        // std::cout << "Ranges: " << rangeHeader << " | File size: " << fz << std::endl;
        // for(auto&& range : ranges) {
        //     std::cout << "\tstart: " << range.start << " -> end: " << range.end << std::endl;
        // }

        std::string rangesStringOut = getRangeStringForResponse(ranges[0], fz);

        // Check file health before reading it
        if (!fileReader->good()) {
            // Maybe it reached the end before?
            fileReader->clear(); // clear eof or bad state
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

        // always write status first
        res->writeStatus("206 Partial Content")
            ->writeHeader("Content-Range", std::string_view(rangesStringOut.data(), rangesStringOut.length()))
            ->writeHeader("Content-Length", fz)
            ->writeHeader("Connection", "keep-alive")
            ->writeHeader("Accept-Ranges", "bytes")
            ->writeHeader("Content-Type", getContentType(fileReader->getFileName()))
            ->writeHeader("Last-Modified", "Thu, 17 Jun 2021 20:50:11 GMT") // TODO: set actual modified time
            ->tryEndRaw(std::string_view(nullptr, 0), 0);            

        res->onAborted([url]() {
            std::cout << "[" << url << "] ABORTED!" << std::endl;
        });

        std::string ran;
        std::string buf; buf.resize(ReadWriteBufferSize);

        for (auto& range : ranges) {
            // std::cout << "\tstart: " << range.start << " -> end: " << range.end << std::endl;

            fileReader->seek(range.start, fileReader->beg());  
            long total = range.start;

            // if we would support multiple ranges we need to send this:
            // res ->writeStatus("206 Partial Content")
            //     ->writeHeader("Content-Range", getRangeStringForResponse(range, fz));

            auto bytesLeft = range.length();
            while (bytesLeft) {
                auto of = std::min(buf.length(), bytesLeft);
                total += of;

                // ran = "bytes 0-" + std::to_string(total) + "/" + std::to_string(fz);
                // std::cout << "Reading " << of << " bytes from file." << " Left: " << bytesLeft <<  " Ran: " << ran << std::endl;
                fileReader->read(buf.data(), of);
                if (of <= 0) {
                    const static std::string unexpectedEof("Unexpected EOF");
                    std::cout << "Error reading file: " << (of == 0 ? unexpectedEof : "getLastError") << std::endl;
                    // We can't send an error document as we've sent the header.
                    return false;
                }
                
                bytesLeft -= of;

                if (!res->tryEndRaw(buf, of).first) {
                    // std::cout << "ended range" << std::endl;
                    // The client doesn't want any more data from this range. 
                    // Stop sending it.
                    break;
                }
            }
        }
        
        return true;
    }

    template <bool SSL>
    bool streamChunkedFile(uWS::HttpResponse<SSL> *res, const std::string& url, FileReader *fileReader) {
        /**
         * When the server request a file we answer with 
         *  -   Status: 200 OK
         *  -   Transfer-Encoding: chunked
         * in the first chunk
         * 
         * Then (without closing the connection [end]) we send the chunk as
         * specified here: https://developer.mozilla.org/en-US/docs/Web/HTTP/Headers/Transfer-Encoding#directives
         * 
         * When we reach the end of file then we send an chunk with 0 size,
         * if sucess then the client should have responded with an FIN flag and we of
         * course try to end as the user said.
         * 
         * Tranfer-Enconding chunk doesn't allow to seek for a specific range on the file,
         * so is not a good idea for media files.
        **/

        long fz = fileReader->getFileSize();

        res->onAborted([url]() {
            std::cout << "[" << url << "] ABORTED!" << std::endl;
        });

        // Check file health before reading it
        if (!fileReader->good()) {
            // Maybe it reached the end before?
            fileReader->clear(); // clear eof or bad state
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
        std::string buf; buf.resize(ReadWriteBufferSize);

        fileReader->seek(0, fileReader->beg());
        long total = 0;

        auto bytesLeft = fz;
        while (bytesLeft) {
            auto of = std::min((long)buf.length(), bytesLeft);
            total += of;

            // ran = "bytes " + std::to_string(total) + "/" + std::to_string(fz);
            // std::cout << "Reading " << of << " bytes from file." << " Left: " << bytesLeft <<  " Ran: " << ran << std::endl;
            fileReader->read(buf.data(), of);
            if (of <= 0) {
                const static std::string unexpectedEof("Unexpected EOF");
                std::cout << "Error reading file: " << (of == 0 ? unexpectedEof : "getLastError") << std::endl;
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
    bool streamFile(uWS::HttpResponse<SSL> *res, std::string url, std::string rangeHeader) {
        if (!staticFiles->fileHandlerExists(url)) {
            if (!staticFiles->addFileHandler(url)) {
                std::cout << HTTP_404_NOT_FOUND << " Did not find file: " << url << std::endl;
                res->writeStatus(HTTP_404_NOT_FOUND);
                res->end();
                return false;
            }
        }

        FileReader* reader = staticFiles->getFileHandler(url);
        
        // request is not ranged, send it chunked
        if (rangeHeader.empty()) {
            return streamChunkedFile(res, url, reader);
        } else {
            // there is a range
            return streamRangedFile(res, url, reader, rangeHeader);
        }
    }
};
