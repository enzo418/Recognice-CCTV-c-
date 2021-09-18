# Recognize people on CCTV

# Requeriments	
- Opencv >= 4.5 (was 3.0 before implementing yolov4 nn)

## Compiling
- [jsoncpp](github.com/open-source-parsers/jsoncpp) (jsoncpp-devel) (To decode incoming messages from the telegram api)
    - or compile it:
        1. `git clone github.com/open-source-parsers/jsoncpp`
        2. `cd jsoncpp && mkdir build && cd build`
        3. `cmake -DCMAKE_BUILD_TYPE=release -DJSONCPP_LIB_BUILD_STATIC=ON-DJSONCPP_LIB_BUILD_SHARED=OFF -G "Unix Makefiles" ..`
        4. `make`
        5. `sudo make install`


# Todo
**1. Comment the code**

**2. Configuration:**
- Allow the user to set a messaging service like dweet.io
