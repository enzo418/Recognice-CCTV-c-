#pragma once
#include <filesystem>
#include "Middleware.h"
#include "Range.h"

const char *HTTP_404_NOT_FOUND = "404 Not Found";
const char *HTTP_301_MOVED_PERMANENTLY = "301 Moved Permanently";

// Max buffer size: 16.384 kb
constexpr size_t ReadWriteBufferSize = 16 * 1024;

struct AsyncFileStreamer {

    std::map<std::string_view, AsyncFileReader *> asyncFileReaders;
    std::string video_test;
    std::string root;

    AsyncFileStreamer(std::string root) : root(root) {
        // for all files in this path, init the map of AsyncFileReaders
        updateRootCache();

        video_test.resize(ReadWriteBufferSize);
    }

    void addToKnownFiles(std::string url, std::filesystem::__cxx11::directory_entry p) {
        char *key = new char[url.length()];
        memcpy(key, url.data(), url.length());

        asyncFileReaders[std::string_view(key, url.length())] = new AsyncFileReader(p.path().string());
    }

    void updateRootCache() {
        // todo: if the root folder changes, we want to reload the cache
        for(auto &p : std::filesystem::recursive_directory_iterator(root)) {
            std::string url = p.path().string().substr(root.length());
            if (url == "/index.html") {
                url = "/";
                // allow to go to /index.html
		        addToKnownFiles("/index.html", p);
            }
            std::cout << "url: " << url << " path: " << p.path().string() << std::endl;
	        addToKnownFiles(url, p);
        }
    }

    template <bool SSL>
    bool streamRangedFile(uWS::HttpResponse<SSL> *res, std::string_view url, std::string rangeHeader, const std::string& server_path) {
        /**
         * This function stream a ranged file to the client.
         * First it tells the client that the content is a Partial content
         * that accepts a range from 0 to the file size.
         * Content type is text/html and not a video/audio since
         * in this response we don't send actually any data of the file.
         * 
         * Then we start sending to the user the actual file data.
        **/

        auto it = asyncFileReaders.find(url);
        AsyncFileReader *fileReader = it->second;
        std::string rangesStringOut;

        std::list<Range> ranges;

        std::cout << "Header string: " << rangeHeader << std::endl;

        if (!rangeHeader.empty() && !parseRanges(rangeHeader, ranges)) {
            // return sendBadRequest("Bad range header");
            std::cout << "Bad request header" << std::endl;
            return false;
        }

        long fz = fileReader->getFileSize();

        ranges = processRangesForStaticData(ranges, fz, rangesStringOut);

        std::cout << "Ranges: \n";
        for(auto&& range : ranges) {
            std::cout << "\tstart: " << range.start << " -> end: " << range.end << std::endl;
        }

        res->writeStatus("206 Partial Content")
            ->writeHeader("Content-Range", std::string_view(rangesStringOut.data(), rangesStringOut.length()))
            ->writeHeader("Content-Length", fz)
            ->writeHeader("Connection", "keep-alive")
            ->writeHeader("Accept-Ranges", "bytes")
            ->writeHeader("Content-Type", "text/html")
            ->writeHeader("Last-Modified", "Thu, 17 Jun 2021 20:50:11 GMT") // TODO: set actual modified time
            ->tryEndRaw(std::string_view(nullptr, 0), 0);            

        std::cout << "Fz: " << fz << std::endl;

        res->onAborted([url]() {
            std::cout << "[" << url << "] ABORTED!" << std::endl;
        });

        std::ifstream* fin = fileReader->getFileHandler();
        
        if (!fin->good()) {
            fin->close();
            std::cout << "Reopening fin!" << std::endl;
            fin->open(fileReader->getFileName(), std::ios::binary);

            std::cout << "File is good? " << (fin->good() ? "yes" : "no") << std::endl;
        }

        std::string ran;
        std::string buf; buf.resize(ReadWriteBufferSize);

        for (auto& range : ranges) {
            std::cout << "\tstart: " << range.start << " -> end: " << range.end << std::endl;

            fin->seekg(range.start, fin->beg);  
            long total = range.start;

            auto bytesLeft = range.length();
            while (bytesLeft) {
                auto of = std::min(buf.length(), bytesLeft);
                total += of;

                ran = "bytes 0-" + std::to_string(total) + "/" + std::to_string(fz);
                std::cout << "Reading " << of << " bytes from file." << " Left: " << bytesLeft <<  " Ran: " << ran << std::endl;
                fin->read(buf.data(), of);
                if (of <= 0) {
                    const static std::string unexpectedEof("Unexpected EOF");
                    std::cout << "Error reading file: " << (of == 0 ? unexpectedEof : "getLastError") << std::endl;
                    // We can't send an error document as we've sent the header.
                    return false;
                }
                
                // if we would support multiple ranges we need to send this:
                // res ->writeStatus("206 Partial Content")
                //     ->writeHeader("Content-Range", std::string_view(ran.data(), ran.length()))
                //     ->writeHeader("Content-Length", of);

                bytesLeft -= of;

                if (!res->tryEndRaw(buf, of).first) {
                    std::cout << "ended range" << std::endl;
                    // The client doesn't want any more data from this range. 
                    // Stop sending it.
                    break;
                }
            }
        }
    }

    template <bool SSL>
    bool streamChunkedFile(uWS::HttpResponse<SSL> *res, std::string_view url, std::string rangeHeader) {
        /**
         * When the server request the video file we answer with 
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

        auto it = asyncFileReaders.find(url);
        AsyncFileReader *fileReader = it->second;

        long fz = fileReader->getFileSize();

        std::cout << "Fz: " << fz << std::endl;

        std::cout << std::endl << std::endl;

        res->onAborted([url]() {
            std::cout << "[" << url << "] ABORTED!" << std::endl;
        });

        std::ifstream* fin = fileReader->getFileHandler();
        
        if (!fin->good()) {
            fin->close();
            std::cout << "Reopening fin!" << std::endl;
            fin->open(fileReader->getFileName(), std::ios::binary);

            std::cout << "File is good? " << (fin->good() ? "yes" : "no") << std::endl;
        }

        std::string ran;
        std::string buf; buf.resize(ReadWriteBufferSize);

        fin->seekg(0, fin->beg);  
        long total = 0;

        auto bytesLeft = fz;
        while (bytesLeft) {
            auto of = std::min((long)buf.length(), bytesLeft);
            total += of;

            ran = "bytes " + std::to_string(total) + "/" + std::to_string(fz);
            std::cout << "Reading " << of << " bytes from file." << " Left: " << bytesLeft <<  " Ran: " << ran << std::endl;
            fin->read(buf.data(), of);
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
                std::cout << "Client ended range" << std::endl;
            }
        }
        
        std::cout << "Sending 0 size chunk\n";
        if (!res->writeOrZero(std::string_view(nullptr, 0))) {
            // res->close();
            // res->end();
            std::cout << "Error sending 0 size chunk\n";
            return false;
        }

        std::cout << "Send end\n" << std::endl;
        res->tryEnd(std::string_view(nullptr, 0), 0);
        return true;
    }

    template <bool SSL>
    bool sendChunk(uWS::HttpResponse<SSL> *res, std::string_view url, AsyncFileReader *asyncFileReader, std::list<Range> ranges, std::string& rangesStringOut) {

        // https://developer.mozilla.org/en-US/docs/Web/HTTP/Range_requests

        // int offset = asyncFileReader->getFileSize();

        // std::cout << "1 [" << url << "] Write offset: " << offset << std::endl;
        std::ifstream* fin = asyncFileReader->getFileHandler();
        
        if (!fin->good()) {
            fin->close();
            std::cout << "Reopening fin!" << std::endl;
            fin->open(asyncFileReader->getFileName(), std::ios::binary);

            std::cout << "File is good? " << (fin->good() ? "yes" : "no") << std::endl;
        }
        
        // fin->seekg(offset, fin->beg);

        // int chunkSize = std::min<int>(video_test.length(), asyncFileReader->getFileSize() - offset);

        // std::cout << "Video_Test.length() == " << video_test.length() << " | filesize: " << asyncFileReader->getFileSize() << " | fsz - offset == " << asyncFileReader->getFileSize() - offset << std::endl;
        // std::cout << "Reading " << chunkSize << " bytes from file." << std::endl;

        // video_test.clear();

        // fin->read(video_test.data(), chunkSize);

        // std::string_view chunk = asyncFileReader->peek(offset);
        // std::string_view chunk(video_test.data(), chunkSize);
        
        // res->writeStatus("206 Partial Content");
            // ->writeHeader("Content-Range", "bytes 0-1023/146515");
        // if (res->tryEnd(chunk, chunkSize).first) {
        //     std::cout << "1 [" << url << "]" << " sucess" << std::endl;
        // } else {
        //     std::cout << "1 [" << url << "]" << " fail" << std::endl;
        // }

        for (auto& range : ranges) {
            std::cout << "\tstart: " << range.start << " -> end: " << range.end << std::endl;
            // if (::lseek(input, range.start, SEEK_SET) == -1) {
            //     // We've (probably) already sent data.
            //     return false;
            // }

            fin->seekg(range.start, fin->beg);
            // auto fz = fin.tellg();

            auto bytesLeft = range.length();
            while (bytesLeft) {
                char buf[ReadWriteBufferSize];
                auto of = std::min(sizeof(buf), bytesLeft);
                std::cout << "Reading " << of << " bytes from file." << " Left: " << bytesLeft << std::endl;
                fin->read(buf, of);
                if (of <= 0) {
                    const static std::string unexpectedEof("Unexpected EOF");
                    std::cout << "Error reading file: " << (of == 0 ? unexpectedEof : "getLastError") << std::endl;
                    // We can't send an error document as we've sent the header.
                    return false;
                }

                // res ->writeStatus("206 Partial Content")
                //     ->writeHeader("Content-Range", std::string_view(rangesStringOut.data(), rangesStringOut.length()))
                //     ->writeHeader("Content-Length", of);

                bytesLeft -= of;
                if (!res->tryEndRaw(buf, of).first) {
                    std::cout << "ended range" << std::endl;
                    return false;
                }
            }
        }
        
        // std::cout << "ended file" << std::endl;
        // res->tryEnd(std::string_view(nullptr, 0), 0);
    }

    template <bool SSL>
    bool streamFile(uWS::HttpResponse<SSL> *res, std::string_view url) {
        bool found = false;
        auto it = asyncFileReaders.find(url);
        if (it == asyncFileReaders.end()) {
            std::cout << HTTP_404_NOT_FOUND << " Did not find file: " << url << std::endl;
            res->writeStatus(HTTP_404_NOT_FOUND);
            res->end();
            return false;
        } else {
            setFileContentType(res, url);
            found = true;
            streamFile(res, it->second);

            return true;
        }
    }

    template <bool SSL>
    static void streamFile(uWS::HttpResponse<SSL> *res, AsyncFileReader *asyncFileReader) {
        /* Peek from cache */
        std::string_view chunk = asyncFileReader->peek(res->getWriteOffset());
        if (!chunk.length() || res->tryEnd(chunk, asyncFileReader->getFileSize()).first) {
            /* Request new chunk */
            // todo: we need to abort this callback if peer closed!
            // this also means Loop::defer needs to support aborting (functions should embedd an atomic boolean abort or something)

            // Loop::defer(f) -> integer
            // Loop::abort(integer)

            // hmm? no?

            // us_socket_up_ref eftersom vi delar ägandeskapet
            std::cout << "[" << asyncFileReader->getFileName() << "] Write offset: " << res->getWriteOffset() << std::endl;

            if (chunk.length() < asyncFileReader->getFileSize()) {
                asyncFileReader->request(res->getWriteOffset(), [res, asyncFileReader](std::string_view chunk) {
                    // check if we were closed in the mean time
                    //if (us_socket_is_closed()) {
                        // free it here
                        //return;
                    //}

                    /* We were aborted for some reason */
                    if (!chunk.length()) {
                        // todo: make sure to check for is_closed internally after all callbacks!
                        res->close();
                    } else {
                        AsyncFileStreamer::streamFile(res, asyncFileReader);
                    }
                });
            }
        } else {
            /* We failed writing everything, so let's continue when we can */
            res->onWritable([res, asyncFileReader](int offset) {

                // här kan skiten avbrytas!

                AsyncFileStreamer::streamFile(res, asyncFileReader);
                // todo: I don't really know what this is supposed to mean?
                return false;
            })->onAborted([]() {
                std::cout << "ABORTED!" << std::endl;
            });
        }
    }
};
