include(ExternalProject)

set(DEPENDENCY_INSTALL_DIR ${PROJECT_BINARY_DIR}/install)
set(DEPENDENCY_INCLUDE_DIR ${DEPENDENCY_INSTALL_DIR}/include)
set(DEPENDENCY_LIB_DIR ${DEPENDENCY_INSTALL_DIR}/lib)

# ------------------ HELPER FUNCTIONS ------------------ #
function(add_dep dep)
    set(DEPENDENCY_LIST ${DEPENDENCY_LIST} ${dep} PARENT_SCOPE)
endfunction()

function(add_include include_dir)
    # add / at the end or else cmake will think that we want to copy the parent folder
    if (NOT (${include_dir} MATCHES ".*/$"))
        set(include_dir "${include_dir}/")
    endif()

    set(DEPENDENCY_INCLUDE_LIST ${DEPENDENCY_INCLUDE_LIST} ${include_dir} PARENT_SCOPE)
endfunction()

function(add_lib lib)
    set(DEPENDENCY_LIBS ${DEPENDENCY_LIBS} ${lib} PARENT_SCOPE)
endfunction(add_lib)

function(add_compilation_option option)
    set(OBSERVER_DEPENDENCY_COMPILATION_OPTIONS ${OBSERVER_DEPENDENCY_COMPILATION_OPTIONS} ${option} PARENT_SCOPE)
endfunction(add_compilation_option)

# ----------------------- OPENCV ----------------------- #
find_package(OpenCV REQUIRED)

add_lib("${OpenCV_LIBS}") # OpenCV_LIBS is a list so quote it

# ----------------------- SPDLOG ----------------------- #
find_package(spdlog REQUIRED)

add_lib(spdlog)

# ------------------------ CURL ------------------------ #
set(CURL_LIBRARY curl)
find_package(CURL REQUIRED)

add_include(${CURL_INCLUDE_DIR})
add_lib("${CURL_LIBRARIES}")

# -------------------- NLOHMANN JSON ------------------- #
ExternalProject_Add(
    dep-json
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/Observer/vendor/json
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CMAKE_ARGS
        -DCMAKE_INSTALL_PREFIX=${DEPENDENCY_INSTALL_DIR}
        -DNLOHMANN_JSON_INCLUDE_INSTALL_DIR=${DEPENDENCY_INSTALL_DIR}
        -DJSON_Install=ON
        -DJSON_BuildTests=OFF
    TEST_COMMAND ""
)

add_dep(dep-json)
add_include(${CMAKE_CURRENT_LIST_DIR}/Observer/vendor/json/include)

# --------------------- MAGIC ENUM --------------------- #
add_include(${CMAKE_CURRENT_LIST_DIR}/Observer/vendor/magic_enum/include)

# ------------------- ASYNC INFERENCE ------------------ #
include(${CMAKE_CURRENT_LIST_DIR}/Observer/src/observer/AsyncInference/module.cmake)

add_dep("${AsyncInference_DEPENDENCIES}")
add_lib("${AsyncInference_LIBS}")
add_include("${AsyncInference_INCLUDE_DIRS}")

# ---------------------- PALANTEER --------------------- #
ExternalProject_Add(
    palanteer
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/Observer/vendor/palanteer
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CMAKE_ARGS
        -DPALANTEER_BUILD_VIEWER=ON
        -DPALANTEER_BUILD_CPP_EXAMPLE=OFF
        -DPALANTEER_BUILD_PYTHON_INSTRUMENTATION=OFF
        -DPALANTEER_BUILD_SERVER_SCRIPTING=OFF
        -DCMAKE_BUILD_TYPE="Release"
    TEST_COMMAND ""
    INSTALL_COMMAND cmake -E echo "Skipping install step."
)

add_dep(palanteer)
add_include(${CMAKE_CURRENT_LIST_DIR}/Observer/vendor/palanteer/c++)
add_compilation_option(-DUSE_PL=1)


# if stacktrace:
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_LIST_DIR}/Observer/vendor/palanteer/cmake") # palanteer custom cmake modules

if(WIN32)
    add_compilation_option(-DPL_IMPL_STACKTRACE=1)
else()
    find_package(LibUnwind)
    find_package(LibDw)
    if (LIBUNWIND_FOUND AND LIBDW_FOUND)
        add_compilation_option(-DPL_IMPL_STACKTRACE=1)
        add_lib(${LIBUNWIND_LIBRARY})
        add_lib(${LIBDW_LIBRARY})
        message(STATUS "Stacktrace enabled")
    else()
        message(WARNING "LibUnwind or LibDw not found. Stacktrace will not be available.")
    endif()
endif(WIN32)
# endif stacktrace

# copy gui tool
add_custom_command(
    TARGET palanteer
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
            ${CMAKE_CURRENT_BINARY_DIR}/palanteer-prefix/src/palanteer-build/bin/palanteer
            ${CMAKE_CURRENT_BINARY_DIR}/gui-instrumentation
)
