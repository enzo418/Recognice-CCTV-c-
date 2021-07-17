#pragma once
#include <filesystem>
#include "Middleware.h"
#include "Range.h"

const char *HTTP_404_NOT_FOUND = "404 Not Found";
const char *HTTP_301_MOVED_PERMANENTLY = "301 Moved Permanently";

// 16.384kb
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
    bool streamRangedFile(uWS::HttpResponse<SSL> *res, std::string_view url, std::string rangeHeader) {
        auto it = asyncFileReaders.find(url);
        AsyncFileReader *fileReader = it->second;
        std::string rangesStringOut;

        std::list<Range> ranges;

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

        std::cout << "Fz: " << fz << std::endl;
        

        // res->onWritable([&](int offset) {
        //     std::cout << "[" << url << "] Writable called, offset: " << offset << std::endl;
        //     sendChunk(res, url, fileReader, ranges, rangesStringOut);
        //     return true;
        // });

        std::cout << std::endl << std::endl;

        res->onAborted([url]() {
            std::cout << "[" << url << "] ABORTED!" << std::endl;
        });

        res->writeStatus("206 Partial Content")
            ->writeHeader("Content-Range", std::string_view(rangesStringOut.data(), rangesStringOut.length()))
            ->writeHeader("Content-Length", fz)
            ->writeHeader("Connection", "keep-alive")
            ->writeHeader("Accept-Ranges", "bytes")
            ->writeHeader("Content-Type", "video/mp4")
            ->writeHeader("last-modified", "Thu, 17 Jun 2021 20:50:11 GMT") // test
            ->tryEndRaw(std::string_view(nullptr, 0), 0);

        std::ifstream* fin = fileReader->getFileHandler();
        
        if (!fin->good()) {
            fin->close();
            std::cout << "Reopening fin!" << std::endl;
            fin->open(fileReader->getFileName(), std::ios::binary);

            std::cout << "File is good? " << (fin->good() ? "yes" : "no") << std::endl;
        }

        std::string ran;
        long total = 0;

        for (auto& range : ranges) {
            std::cout << "\tstart: " << range.start << " -> end: " << range.end << std::endl;

            fin->seekg(range.start, fin->beg);            

            auto bytesLeft = range.length();
            while (bytesLeft) {
                char buf[ReadWriteBufferSize];
                auto of = std::min(sizeof(buf), bytesLeft);
                total += of;

                ran = "bytes 0-" + std::to_string(total) + "/" + std::to_string(fz);
                std::cout << "Reading " << of << " bytes from file." << " Left: " << bytesLeft <<  " Ran: " << ran << std::endl;
                fin->read(buf, of);
                if (of <= 0) {
                    const static std::string unexpectedEof("Unexpected EOF");
                    std::cout << "Error reading file: " << (of == 0 ? unexpectedEof : "getLastError") << std::endl;
                    // We can't send an error document as we've sent the header.
                    return false;
                }

                // res ->writeStatus("206 Partial Content")
                //     ->writeHeader("Content-Range", std::string_view(ran.data(), ran.length()))
                //     ->writeHeader("Content-Length", of);

                bytesLeft -= of;
                if (!res->tryEndRaw(buf, of).first) {
                    std::cout << "ended range" << std::endl;
                    return false;
                }
            }
        }

        // res->tryEnd(std::string_view(nullptr, 0), 0);
        // res->close();
        // res->end();
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
