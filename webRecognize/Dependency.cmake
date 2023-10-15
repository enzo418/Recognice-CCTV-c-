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

# -------------------------------------------------------- #
#                 NOTES ABOUT DEPENDENCIES                 #
# -------------------------------------------------------- #

# ---------------- ABOUT BORING SSL/CRYPTO --------------- #
# You might notice that I avoid linking to boringssl in grpc
# and any other lib that uses it. This is because webrtc
# already includes boringssl in the static library. This could
# be easily solved if webrtc was compiled as a shared library,
# but you can't. Neither can grpc, at least it's not "recomended".
# The easy fix was to use grpc with the "_unsecure" tag, which
# doesn't links against boringssl, crypto. But with more time
# one could also compile webrtc without boringssl and do all here.

# -------------------------------------------------------- #
#                       DEPENDENCIES                       #
# -------------------------------------------------------- #


# -------------- OBSERVER LIB / RECOGNIZE -------------- #
set(IFC_USE_INSECURE_GRPC ON) # SSL, CRYPTO objects are included in libwebrtc

include(../recognize/CMakeLists.txt)
web_add_lib(recognize)
# web_add_include(${OBSERVER_INCLUDE_DIRS})

# --------------------- uWebSockets -------------------- #
add_subdirectory(vendor/uWebSockets)

# TODO: Include it as an external project
# ExternalProject_Add(
#     uWebSockets-dep
#     SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/uWebSockets
#     UPDATE_COMMAND ""
#     PATCH_COMMAND ""
#     CMAKE_ARGS 
#         -DCMAKE_INSTALL_PREFIX=${DEPENDENCY_INSTALL_DIR}
#         -DBUILD_EXAMPLES=OFF
#         -DBUILD_SHARED_LIBS=ON # Avoid potentiallinking errors with webrtc, 
#                                # which includes boringssl lib
#                                # TODO: Make boing SSL link optional
#         -DWITH_OPENSSL=ON
#         -DWITH_WOLFSSL=OFF
# )
# web_add_dep(uWebSockets-dep)

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

# ------------------------ WEBRTC ------------------------ #
if (${WITH_WEBRTC})
    ExternalProject_Add(
        web-rtc-build
        SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor/webrtc-build-helpers
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${DEPENDENCY_INSTALL_DIR}
            -DCMAKE_INSTALL_INCLUDEDIR=${DEPENDENCY_INCLUDE_DIR}
            -DCMAKE_INSTALL_LIBDIR=${DEPENDENCY_LIB_DIR}
            -DWEBRTC_BUILD_TYPE=Release
            -DWEBRTC_INCLUDE_DEFAULT_AUDIO=OFF
            -DWEBRTC_BUILD_SSL=OFF
        TEST_COMMAND ""
    )

    add_dep(web-rtc-build)
    add_lib(webrtc)
    add_lib(webrtc_extra)

    add_include(${DEPENDENCY_INSTALL_DIR}/include/webrtc) # webrtc source uses api/, modules/, ...
endif()