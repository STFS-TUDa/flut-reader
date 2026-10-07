cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

include(FetchContent)

project(flut VERSION 1.0.0 LANGUAGES CXX)

# Set the cmake options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set a default installation prefix if none is provided
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX
      "${CMAKE_BINARY_DIR}/../install"
      CACHE PATH "Default install prefix" FORCE)
endif()
message(STATUS "Install prefix is set to ${CMAKE_INSTALL_PREFIX}")

# Define options
option(BUILD_WITH_TESTING "Build the library with testing" OFF)

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/src)

# Compiler options
add_compile_options(-Wunused-variable -Wunreachable-code)

# Preprocessor definitions
add_compile_definitions(DATA_DIR="${PROJECT_SOURCE_DIR}/data/")

# Link options
add_library(
  ${PROJECT_NAME} SHARED
  src/containers.cpp
  src/interpolator.cpp
  src/lookup_table.cpp
  )

# Target include directories
target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}> ${PROJECT_SOURCE_DIR}/src
  INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Installation directories
install(
  TARGETS ${PROJECT_NAME}
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)

install(DIRECTORY ${PROJECT_SOURCE_DIR}/src/ DESTINATION include)

# Instal HighFive as external dependency
# Additionally, the CMAKE warnings from HighFive are ignored.
FetchContent_Declare(
  HighFive
  GIT_REPOSITORY https://github.com/highfive-devs/highfive
  GIT_TAG v3.2.0
)
# Disable unwanted stuff from highfive
set(HIGHFIVE_EXAMPLES OFF)
set(HIGHFIVE_UNIT_TESTS OFF)
set(HIGHFIVE_USE_BOOST OFF)
set(HIGHFIVE_BUILD_DOCS OFF)
# Fetch and include highfive
FetchContent_MakeAvailable(HighFive)
# Link HighFive library
# librt (POSIX realtime extensions) is Linux-only; on macOS those symbols are part of libc
if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE HighFive)
else()
    target_link_libraries(${PROJECT_NAME} PRIVATE HighFive -lrt)
endif()

# Default build type
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE
      Release
      CACHE STRING "Choose the type of build." FORCE)
  message(STATUS "Default build type set to ${CMAKE_BUILD_TYPE}")
endif()

# Testing
if(BUILD_WITH_TESTING)

  enable_testing()
  add_subdirectory(test)

endif()

# We could also use the following to fetch HDF5, but this means that we compile
# HDF5 during each fresh build Unlike googletest, it's easily available in
# common repositories, so for now can can just require it to be installed
# FetchContent_Declare( hdf5 GIT_REPOSITORY https://github.com/HDFGroup/hdf5.git
# GIT_TAG        hdf5-1_14_3 # specify a particular version/tag )
#
# FetchContent_GetProperties(hdf5) if(NOT hdf5_POPULATED)
# FetchContent_Populate(hdf5) add_subdirectory(${hdf5_SOURCE_DIR}
# ${hdf5_BINARY_DIR}) endif()

# creates coverage reports if (CMAKE_BUILD_TYPE MATCHES "Debug")
# target_compile_options(${PROJECT_NAME} PUBLIC -fprofile-instr-generate
# -fcoverage-mapping) target_link_options(${PROJECT_NAME} PUBLIC
# -fprofile-instr-generate -fcoverage-mapping)
#
# # create simple report: make coverage add_custom_command(OUTPUT "coverage
# report" COMMAND llvm-profdata merge --sparse
# ${PROJECT_BINARY_DIR}/coverage/*.profraw -o
# ${PROJECT_BINARY_DIR}/coverage/coverage.profdata COMMAND llvm-cov report
# ./lib${PROJECT_NAME}.so
# --instr-profile=${PROJECT_BINARY_DIR}/coverage/coverage.profdata COMMAND rm
# ${PROJECT_BINARY_DIR}/coverage/coverage.profdata ) add_custom_target(coverage
# DEPENDS "coverage report")
#
# # create detailed report: make coverage_detail add_custom_command(OUTPUT
# "detailed coverage" COMMAND llvm-profdata merge --sparse
# ${PROJECT_BINARY_DIR}/coverage/*.profraw -o
# ${PROJECT_BINARY_DIR}/coverage/coverage.profdata COMMAND llvm-cov show
# ./lib${PROJECT_NAME}.so
# --instr-profile=${PROJECT_BINARY_DIR}/coverage/coverage.profdata COMMAND rm
# ${PROJECT_BINARY_DIR}/coverage/coverage.profdata )
# add_custom_target(coverage_detail DEPENDS "detailed coverage") endif()
