include(ExternalProject)

set(DEPENDENCY_INSTALL_DIR ${PROJECT_BINARY_DIR}/install)
set(DEPENDENCY_INCLUDE_DIR ${DEPENDENCY_INSTALL_DIR}/include)
set(DEPENDENCY_LIB_DIR ${DEPENDENCY_INSTALL_DIR}/lib)

# ------------------ HELPER FUNCTIONS ------------------ #
function(web_add_dep dep)
    set(DEPENDENCY_LIST ${DEPENDENCY_LIST} ${dep} PARENT_SCOPE)
endfunction()

function(web_add_include include_dir)
    # add / at the end or else cmake will think that we want to copy the parent folder
    if (NOT (${include_dir} MATCHES ".*/$"))
        set(include_dir "${include_dir}/")
    endif()

    set(DEPENDENCY_INCLUDE_LIST ${DEPENDENCY_INCLUDE_LIST} ${include_dir} PARENT_SCOPE)
endfunction()

function(web_add_lib lib)
    set(DEPENDENCY_LIBS ${DEPENDENCY_LIBS} ${lib} PARENT_SCOPE)
endfunction(web_add_lib)

# -------------- OBSERVER LIB / RECOGNIZE -------------- #
include(../recognize/CMakeLists.txt)
web_add_lib(recognize)
# web_add_include(${OBSERVER_INCLUDE_DIRS})

# --------------------- uWebSockets -------------------- #
# TODO: Include it as an external project
add_subdirectory(vendor/uWebSockets)

web_add_include(vendor/uWebSockets/src)
web_add_include(vendor/uWebSockets/uSockets/src)

web_add_lib(usocketslib)

# it requires zlib
find_package(ZLIB REQUIRED)
web_add_lib("${ZLIB_LIBRARIES}") # -lz

# ---------------------- NOLITEDB ---------------------- #
set(__BUILD_NOLITE_SHARED ON)

ExternalProject_Add(
    nolitedb-dep
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/nolitedb
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    BUILD_ALWAYS 1
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX=${DEPENDENCY_INSTALL_DIR}
		-DCMAKE_BUILD_TYPE=DEBUG
		-DNLDB_INSTALL=ON
		-DNLDB_LOGGING=ON
		-DNLDB_BUILD_SHARED=${__BUILD_NOLITE_SHARED}
        -DNLDB_INTERNAL_ID=_id
        -DNLDB_ENABLE_DOUBLE_DOWNCASTING=ON
    TEST_COMMAND ""
)

web_add_include(${DEPENDENCY_INSTALL_DIR}/include/)
web_add_dep(nolitedb-dep)
web_add_lib(nolitedb)

if (NOT ${__BUILD_NOLITE_SHARED})
    web_add_lib(xsqllite3)
endif()