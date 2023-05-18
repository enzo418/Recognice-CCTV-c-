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
    set(IFC_DEPENDENCY_COMPILATION_OPTIONS ${IFC_DEPENDENCY_COMPILATION_OPTIONS} ${option} PARENT_SCOPE)
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