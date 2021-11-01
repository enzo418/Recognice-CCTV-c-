# Recognize people on CCTV

# Requeriments	
- Opencv >= 4.5 (or 3.0 without yolov4 nn)
- jsoncpp
- restclient-cpp

## Compiling
- [jsoncpp](github.com/open-source-parsers/jsoncpp) (jsoncpp-devel) (To decode incoming messages from the telegram api)
    - or compile it:
        1. `git clone github.com/open-source-parsers/jsoncpp`
        2. `cd jsoncpp && mkdir build && cd build`
        3. `cmake -DCMAKE_BUILD_TYPE=release -DJSONCPP_LIB_BUILD_STATIC=ON-DJSONCPP_LIB_BUILD_SHARED=OFF -G "Unix Makefiles" ..`
        4. `make`
        5. `sudo make install`

- [restclient-cpp](https://github.com/mrtazz/restclient-cpp) Used to conect to the web server
        1. `git clone https://github.com/mrtazz/restclient-cpp`
        2. `cd restclient-cpp && mkdir build && cd build`
        3. `cmake ..`
        4. `make -j4`
        5. `sudo make install`
