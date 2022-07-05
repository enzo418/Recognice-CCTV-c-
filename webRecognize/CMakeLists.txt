cmake_minimum_required(VERSION 3.12)
project(WebRecognize VERSION 1.0.0)

# We don't support Windows or Apple platforms since seasocks doesn't.
if(WIN32)
    message(FATAL_ERROR "${PROJECT_NAME} does not support Windows")
endif()
if(APPLE)
    message(FATAL_ERROR "${PROJECT_NAME} does not support Apple platforms")
endif()

option(WITH_CUDA "Use cuda DNN" ON)
option(SKIP_INSTALL_FRONTEND_PACKAGES "Skip the installation of all the frontend packages before build" OFF)
option(BUILD_FRONTEND "Should compile the frontend code" ON)

if (WITH_CUDA)
	MESSAGE(STATUS "WITH_CUDA")
	add_definitions(-DWITH_CUDA)
else()
	MESSAGE(STATUS "NO CUDA")
endif()


# message(SPDLOG_FMT_EXTERNAL ${SPDLOG_FMT_EXTERNAL})
# if(SPDLOG_FMT_EXTERNAL)
#   add_compile_definitions(SPDLOG_FMT_EXTERNAL)
#   add_definitions(-DSPDLOG_FMT_EXTERNAL)
#   set(SPDLOG_FMT_EXTERNAL true)
# endif()

include(../recognize/CMakeLists.txt)

# clangd:
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(source_dir "${PROJECT_SOURCE_DIR}")

file(GLOB COMMON_SRC "${source_dir}/src/*.cpp")

file(GLOB STREAM_CONTENT_SRC "${source_dir}/src/stream_content/*.cpp")

file(GLOB NOTIFICATIONS_SR "${source_dir}/src/Notifications/*.cpp")

file(GLOB DOMAIN_SR "${source_dir}/src/Domain/*.cpp")

file(GLOB DTO_SR "${source_dir}/src/DTO/*.cpp")

file(GLOB DAL_SR "${source_dir}/src/DAL/*.cpp" "${source_dir}/src/DAL/InMemory/*.cpp")

file(GLOB CONTROLLER_SR "${source_dir}/src/Controller/*.cpp")

file(GLOB CL_SR "${source_dir}/src/CL/*.cpp")

# set(EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/build/")

set(SOURCES 
	${DTO_SR}
	${COMMON_SRC} 
	${NOTIFICATIONS_SR} 
	${STREAM_CONTENT_SRC} 
	${DOMAIN_SR} 
	${DAL_SR} 
	${CONTROLLER_SR} 
	${CL_SR})

# Print sources
# message(${SOURCES})
# message(${STREAM_CONTENT_SRC})

# Set cmake cxx flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -pthread -lX11")

add_executable(webRecognize ${SOURCES})

# set -l of opencv and curl 
# add -lssl if use ssl with uwebsocket
# -lz is zlib
target_link_libraries(webRecognize ${RECOGNIZER_LIBS} "-l:uSockets.a" "-lz")


## Build npm webpage
file(TO_CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/WebRecognize WebDirectory)

file(TO_CMAKE_PATH ${CMAKE_BINARY_DIR}/web WebOutputDirectory)

if(BUILD_FRONTEND)
	message(STATUS "Web source directory: " ${WebDirectory})
	message(STATUS "Web output directory: " ${WebOutputDirectory})

	if(NOT SKIP_INSTALL_FRONTEND_PACKAGES)
		add_custom_target (npm-target ALL
			COMMAND npm install -s
			WORKING_DIRECTORY ${WebDirectory}
			COMMENT "Installing packges of the frontend"
		)
	endif()

	# WebRecognize npm target
	add_custom_target (npm-target ALL
		COMMAND npm run watch -q -- -b=${WebOutputDirectory} -c -v=0
		WORKING_DIRECTORY ${WebDirectory}
		COMMENT "Building the frontend"
	)
endif()
