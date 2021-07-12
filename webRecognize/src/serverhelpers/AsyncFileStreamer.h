#pragma once
#include <filesystem>

const char *HTTP_404_NOT_FOUND = "HTTP/1.1 404 Not Found";

struct AsyncFileStreamer {

    std::map<std::string_view, AsyncFileReader *> asyncFileReaders;
    std::string root;

    AsyncFileStreamer(std::string root) : root(root) {
        // for all files in this path, init the map of AsyncFileReaders
        updateRootCache();
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
    bool streamFile(uWS::HttpResponse<SSL> *res, std::string_view url) {
        bool found = false;
        auto it = asyncFileReaders.find(url);
        if (it == asyncFileReaders.end()) {
            std::cout << HTTP_404_NOT_FOUND << " Did not find file: " << url << std::endl;
            res->writeStatus(std::string_view(HTTP_404_NOT_FOUND));
            // res->writeHeader("Connection", "close");
            // res->close();
            res->end();
            return false;
        } else {
            // res->writeStatus(uWS::HTTP_200_OK);
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