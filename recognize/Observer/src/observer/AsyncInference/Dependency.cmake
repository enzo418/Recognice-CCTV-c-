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

# ------------------------ GRPC ------------------------ #
if (IFC_LINK_GRPC)
    if (IFC_USE_EXTERNAL_GRPC)
        set(protobuf_MODULE_COMPATIBLE TRUE)

        find_package(Protobuf CONFIG)
        find_package(gRPC CONFIG)

        get_target_property(gRPC_INSTALL_INCLUDEDIR gRPC::grpc INTERFACE_INCLUDE_DIRECTORIES)

        if (NOT gRPC_INSTALL_INCLUDEDIR)
            message(FATAL_ERROR "Unable to find gRPC includes for this version (${gRPC_VERSION})")
        endif()
        
        message(STATUS "Using preinstalled gRPC: Protobuf version ${Protobuf_VERSION}, gRPC version ${gRPC_VERSION}, include=${gRPC_INSTALL_INCLUDEDIR}")

        if(NOT gRPC_FOUND OR NOT Protobuf_FOUND)
            message(FATAL_ERROR "gRPC not found. Please install gRPC or set IFC_USE_EXTERNAL_GRPC to OFF")
        endif()

        if(CMAKE_CROSSCOMPILING)
            find_program(IFC_GRPC_CPP_PLUGIN grpc_cpp_plugin)
        else()
            set(IFC_GRPC_CPP_PLUGIN $<TARGET_FILE:gRPC::grpc_cpp_plugin>)
        endif()

        if(CMAKE_CROSSCOMPILING)
        find_program(IFC_PROTOC_GEN_PROGRAM protoc)
        else()
        set(IFC_PROTOC_GEN_PROGRAM $<TARGET_FILE:protobuf::protoc>)
        endif()

        set(IFC_PROTOBUF_LIBPROTOBUF protobuf::libprotobuf)
        set(IFC_GRPC_REFLECTION gRPC::grpc++_reflection)
        set(IFC_GRPC_GRPCPP gRPC::grpc++)

        ifc_add_lib(${IFC_PROTOBUF_LIBPROTOBUF})
        ifc_add_lib(${IFC_GRPC_REFLECTION})
        ifc_add_lib(${IFC_GRPC_GRPCPP})

        ifc_add_include(${gRPC_INSTALL_INCLUDEDIR})
    else()
        message(STATUS "Using remote gRPC")

        FetchContent_Declare(
            grpc
            GIT_REPOSITORY https://github.com/grpc/grpc.git
            GIT_TAG        v1.54.1
        )
        set(FETCHCONTENT_QUIET OFF)
        FetchContent_MakeAvailable(gRPC)

        # Since FetchContent uses add_subdirectory under the hood, we can use
        # the grpc targets directly from this build.
        set(IFC_PROTOC_GEN_PROGRAM $<TARGET_FILE:protoc>)
        
        if(CMAKE_CROSSCOMPILING)
            find_program(IFC_GRPC_CPP_PLUGIN grpc_cpp_plugin)
        else()
            set(IFC_GRPC_CPP_PLUGIN $<TARGET_FILE:grpc_cpp_plugin>)
        endif()

        set(IFC_PROTOBUF_LIBPROTOBUF libprotobuf)
        set(IFC_GRPC_REFLECTION grpc++_reflection)
        set(IFC_GRPC_GRPCPP grpc++)

        ifc_add_lib(${IFC_PROTOBUF_LIBPROTOBUF})
        ifc_add_lib(${IFC_GRPC_REFLECTION})
        ifc_add_lib(${IFC_GRPC_GRPCPP})

        # includes comes from add_subdirectory
    endif()
endif(IFC_LINK_GRPC)