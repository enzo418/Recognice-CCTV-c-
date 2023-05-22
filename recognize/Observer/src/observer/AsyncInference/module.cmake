cmake_minimum_required(VERSION 3.10)
project(AsyncInference CXX)

# ------------------------- DEV ------------------------ #
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 20)
  set(CMAKE_CXX_STANDARD 20)
endif()

# ----------------------- OPTIONS ---------------------- #
option(IFC_USE_EXTERNAL_GRPC "Use preinstalled grpc" ON)
option(IFC_LINK_GRPC "Link gRPC" ON)

# -------------------- Dependencies -------------------- #
include(${CMAKE_CURRENT_LIST_DIR}/Dependency.cmake)

# ---------------------- Includes ---------------------- #
# include_directories(${IFC_DEPENDENCY_INCLUDE_LIST})
# include_directories(${IFC_DEPENDENCY_INCLUDE_DIR})

# ---------------- Generate proto files ---------------- #
get_filename_component(ifc_proto "${CMAKE_CURRENT_LIST_DIR}/protos/inference.proto" ABSOLUTE)
get_filename_component(ifc_proto_path "${ifc_proto}" PATH)

set(ifc_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/inference.pb.cc")
set(ifc_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/inference.pb.h")
set(ifc_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/inference.grpc.pb.cc")
set(ifc_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/inference.grpc.pb.h")

add_custom_command(
      OUTPUT "${ifc_proto_srcs}" "${ifc_proto_hdrs}" "${ifc_grpc_srcs}" "${ifc_grpc_hdrs}"
      COMMAND ${IFC_PROTOC_GEN_PROGRAM}
      ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${ifc_proto_path}"
        --plugin=protoc-gen-grpc="${IFC_GRPC_CPP_PLUGIN}"
        "${ifc_proto}"
      DEPENDS "${ifc_proto}")

if(PROJECT_IS_TOP_LEVEL)
  # Include generated *.pb.h files
  include_directories("${CMAKE_CURRENT_BINARY_DIR}")
endif()

add_library(ifc_grpc_proto
  ${ifc_grpc_srcs}
  ${ifc_grpc_hdrs}
  ${ifc_proto_srcs}
  ${ifc_proto_hdrs})

target_include_directories(ifc_grpc_proto PUBLIC ${gRPC_INSTALL_INCLUDEDIR})

# ------------------- Link libraries ------------------- #
set(AsyncInference_DEPENDENCIES "" ${IFC_DEPENDENCY_LIST})
set(AsyncInference_INCLUDE_DIRS ${IFC_DEPENDENCY_INCLUDE_LIST})
set(AsyncInference_LIBS ifc_grpc_proto ${IFC_DEPENDENCY_LIBS})
set(AsyncInference_COMPILE_OPTIONS ${IFC_DEPENDENCY_COMPILATION_OPTIONS})

# ----------------------- INSTALL ---------------------- #
install(TARGETS ifc_grpc_proto DESTINATION lib)
install(FILES ${ifc_grpc_hdrs} ${ifc_proto_hdrs} DESTINATION include)
