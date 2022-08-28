# Compiling
- You will need installed on your system
    - compiler for c++ >= 20
    - curl
    - zlib
    - fmt
    - opencv
- [uWebSocket](https://github.com/uNetworking/uWebSockets/) - included - **no need to install it**
    - if compiled with `-DWITH_OPENSSL=ON`, openssl is required to be installed *(system, check uSocket CMakeLists file).*
- [json_dto](https://github.com/Stiffstream/json_dto) - included - **no need to install it**

## Webpage
cmake in the first run will do this: *(but you can disable it with BUILD_FRONTEND=OFF)*
1. `cd ./webRecognize/src/web/`
2. `npm install` - *Cmake option: SKIP_INSTALL_FRONTEND_PACKAGES=OFF|ON*
3. `npm run build` - *Cmake option: BUILD_FRONTEND=OFF|ON*

Finally to compile the server+recognizer run:
1. `cd ./webRecognize && mkdir build && cd /build/`

2. Generate cmake files with all the options:
    2.1. `cmake .. options`
        
    >e.g. cmake .. -DWITH_CUDA=ON -DBUILD_FRONTEND=ON -DWITH_OPENSSL=OFF

    CUDA is currently only necessary if you want to use the yolov4 network with good performance.
3. `make -j`
4. `./webRecognize`

>Optionally you can pass the "--file_path=path" argument to start the recognizer with a file without having to choose it from the web.

# Source folder
## DAL - Data access layer
Repositories interfaces and implementations like in memory, mock and db.

## CL - Cache layer
Currently using [mohaps](https://github.com/mohaps/lrucache11) implementation of the LRU cache algorithm, which is a wrapper of std::unordered_map with a fixed size limit.

- NotificationCL: Notifications are the ideal example to use cache since they don't change once saved, also they can be required several times.

## Domain
