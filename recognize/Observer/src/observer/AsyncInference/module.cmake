cmake_minimum_required(VERSION 3.10)
project(AsyncInference CXX)

# ------------------------- DEV ------------------------ #
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
if (NOT DEFINED CMAKE_CXX_STANDARD OR CMAKE_CXX_STANDARD LESS 20)
  set(CMAKE_CXX_STANDARD 20)
endif()

# -------------------- Dependencies -------------------- #
include(${CMAKE_CURRENT_LIST_DIR}/Dependency.cmake)

# ---------------------- Includes ---------------------- #
# include_directories(${IFC_DEPENDENCY_INCLUDE_LIST})
# include_directories(${IFC_DEPENDENCY_INCLUDE_DIR})

# ------------------- Link libraries ------------------- #
set(AsyncInference_DEPENDENCIES "" ${IFC_DEPENDENCY_LIST})
set(AsyncInference_INCLUDE_DIRS ${IFC_DEPENDENCY_INCLUDE_LIST})
set(AsyncInference_LIBS ${IFC_DEPENDENCY_LIBS})
set(AsyncInference_COMPILE_OPTIONS ${IFC_DEPENDENCY_COMPILATION_OPTIONS})

# ----------------------- INSTALL ---------------------- #
