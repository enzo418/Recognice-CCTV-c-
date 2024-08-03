include(ExternalProject)
include(FetchContent)
include(CMakePushCheckState)

set(IFC_DEPENDENCY_INSTALL_DIR ${PROJECT_BINARY_DIR}/install)
set(IFC_DEPENDENCY_INCLUDE_DIR ${IFC_DEPENDENCY_INSTALL_DIR}/include)
set(IFC_DEPENDENCY_LIB_DIR ${IFC_DEPENDENCY_INSTALL_DIR}/lib)

# ------------------ HELPER FUNCTIONS ------------------ #
function(ifc_add_dep dep)
    set(IFC_DEPENDENCY_LIST ${IFC_DEPENDENCY_LIST} ${dep} PARENT_SCOPE)
endfunction()

function(ifc_add_include include_dir)
    # add / at the end or else cmake will think that we want to copy the parent folder
    if (NOT (${include_dir} MATCHES ".*/$"))
        set(include_dir "${include_dir}/")
    endif()

    set(IFC_DEPENDENCY_INCLUDE_LIST ${IFC_DEPENDENCY_INCLUDE_LIST} ${include_dir} PARENT_SCOPE)
endfunction()

function(ifc_add_lib lib)
    set(IFC_DEPENDENCY_LIBS ${IFC_DEPENDENCY_LIBS} ${lib} PARENT_SCOPE)
endfunction(ifc_add_lib)

function(ifc_add_compilation_option option)
    set(IFC_DEPENDENCY_COMPILATION_OPTIONS ${IFC_DEPENDENCY_COMPILATION_OPTIONS} ${option} PARENT_SCOPE)
endfunction(ifc_add_compilation_option)

# ---------------------- uSockets ---------------------- #
ExternalProject_Add(
    usocket-ain-delp
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/uSockets
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CMAKE_ARGS
        -Dusocket_INSTALL=ON
        -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX=${IFC_DEPENDENCY_INSTALL_DIR}
		-DCMAKE_INSTALL_INCLUDEDIR=${IFC_DEPENDENCY_INCLUDE_DIR}
		-DCMAKE_INSTALL_LIBDIR=${IFC_DEPENDENCY_LIB_DIR}
        -DBUILD_SHARED_LIBS=ON
    TEST_COMMAND ""
)

ifc_add_dep(usocket-ain-delp)
ifc_add_include(${CMAKE_CURRENT_LIST_DIR}/uSockets/src)
ifc_add_lib(usocketslib)

# ------------------------- MDNS ------------------------- #
ExternalProject_Add(
    dep-mdns
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/mdns-cpp
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CMAKE_ARGS
		-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
        -DCMAKE_INSTALL_PREFIX=${IFC_DEPENDENCY_INSTALL_DIR}
		-DCMAKE_INSTALL_INCLUDEDIR=${IFC_DEPENDENCY_INCLUDE_DIR}
		-DCMAKE_INSTALL_LIBDIR=${IFC_DEPENDENCY_LIB_DIR}
		-DMDNS_BUILD_EXAMPLE=OFF
		-DMDNS_DISABLE_LOGGING=ON
    TEST_COMMAND ""
)

ifc_add_dep(dep-mdns)
ifc_add_lib(mdns)