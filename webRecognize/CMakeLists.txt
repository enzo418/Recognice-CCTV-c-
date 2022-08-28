cmake_minimum_required(VERSION 3.12)
project(WebRecognize VERSION 1.0.0)

option(WITH_CUDA "Use cuda DNN" OFF)
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

file(GLOB_RECURSE CPPSRCS "${source_dir}/src/*.cpp")

set(SOURCES ${CPPSRCS})

# Compiler
# -----
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -pthread -lX11")

add_executable(webRecognize ${SOURCES})

# uWebsockets
# ----
include_directories(vendor/uWebSockets/src)

# 	We need to include uSockets again since we are using add_directories.
# 	other alternatives are use target_include_directories or include the
#	cmakelists from uWebSockets
include_directories(vendor/uWebSockets/uSockets/src)

add_subdirectory(vendor/uWebSockets)

# Link libraries
# -----
# 		-lz is zlib
target_link_libraries(webRecognize ${RECOGNIZER_LIBS} usocketslib "-lz")

# Build NPM Web Page
# -----
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
