# Compiling
- You will need installed on your system
    - compiler for c++ >= 20
    - curl
    - zlib
    - fmt
    - opencv
- [uWebSocket](https://github.com/uNetworking/uWebSockets/)
    - already included a modified version
    - [uSocket](https://github.com/uNetworking/uSockets/tree/5440dbac79bd76444175b76ee95dfcade12a6aac)
        - `git clone https://github.com/uNetworking/uSockets/`
        - `cd uSockets && make`
        - `sudo cp uSockets.a /usr/local/lib/`
        - `cd ./src && sudo cp libusockets.h /usr/include/`

- Webpage:
    1. `cd ./webRecognize/src/web/`
    2. `npm install`
    3. `npm run build`

Finally to compile the server+recognizer run:
    
1. `cd ./webRecognize && mkdir build && cd /build/`

2. Generate cmake files with or without cuda:

    2.1. `cmake .. -DWITH_CUDA=ON`

    or

    2.1. `cmake .. -DWITH_CUDA=OFF`

    CUDA is currently only necessary if you want to use the yolov4 network with good performance.
3. `make`
4. `./webRecognize`

>Optionally you can pass the "--file_path=path" argument to start the recognizer with a file without having to choose it from the web.

# Source folder
## DAL - Data access layer
Repositories interfaces and implementations like in memory, mock and db.

## CL - Cache layer
Currently using [mohaps](https://github.com/mohaps/lrucache11) implementation of the LRU cache algorithm, which is a wrapper of std::unordered_map with a fixed size limit.

- NotificationCL: Notifications are the ideal example to use cache since they don't change once saved, also they can be required several times.

## Domain