cmake_minimum_required(VERSION 3.12)
project(WebRecognize VERSION 1.0.0)

# ----------------------- OPTIONS ---------------------- #
option(WITH_CUDA "Use cuda DNN" OFF)
option(SKIP_INSTALL_FRONTEND_PACKAGES "Skip the installation of all the frontend packages before build" OFF)
option(BUILD_FRONTEND "Should compile the frontend code" ON)

if (WITH_CUDA)
	MESSAGE(STATUS "WITH_CUDA")
	add_definitions(-DWITH_CUDA)
else()
	MESSAGE(STATUS "NO CUDA")
endif()

include(Dependency.cmake)

# Include our own files
include_directories(src/)

# clangd:
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# -------------- COMPILATION CONFIGURATION ------------- #
set (CMAKE_CXX_STANDARD 20)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread -lX11")

file(GLOB_RECURSE SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")

add_executable(webRecognize ${SOURCES})

add_dependencies(webRecognize ${DEPENDENCY_LIST})

target_link_directories(webRecognize PUBLIC ${DEPENDENCY_LIB_DIR})
target_include_directories(webRecognize PUBLIC ${DEPENDENCY_INCLUDE_LIST})

# Define build type
target_compile_definitions(webRecognize PUBLIC
							$<$<CONFIG:Debug>:BUILD_DEBUG>
							$<$<CONFIG:RelWithDebInfo>:BUILD_DEBUG>
							$<$<CONFIG:Release>:BUILD_RELEASE>
							$<$<CONFIG:MinSizeRel>:BUILD_RELEASE>
)

target_compile_definitions(webRecognize PUBLIC ${OBSERVER_DEPENDENCY_COMPILATION_OPTIONS})

# Link libraries
# -----
target_link_libraries(webRecognize PUBLIC ${DEPENDENCY_LIBS})

# Build NPM Web Page
# -----
file(TO_CMAKE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/WebRecognize WebDirectory)

file(TO_CMAKE_PATH ${CMAKE_BINARY_DIR}/web WebOutputDirectory)

if(BUILD_FRONTEND)
	message(STATUS "Web source directory: " ${WebDirectory})
	message(STATUS "Web output directory: " ${WebOutputDirectory})


	# WebRecognize npm target
	add_custom_target (npm-target ALL
		COMMAND npm run watch -q -- -b=${WebOutputDirectory} -c -v=0
		WORKING_DIRECTORY ${WebDirectory}
		COMMENT "Building the frontend"
	)

	if(NOT SKIP_INSTALL_FRONTEND_PACKAGES)
		add_custom_target (npm-target-install ALL
			COMMAND npm install -s
			WORKING_DIRECTORY ${WebDirectory}
			COMMENT "Installing packges of the frontend"
		)
		
		add_dependencies(npm-target npm-target-install)
	endif()
endif()
