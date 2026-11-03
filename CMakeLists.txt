cmake_minimum_required(VERSION 3.20)
project(investm VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE RelWithDebInfo CACHE STRING "Build type" FORCE)
endif()

option(INVESTM_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
option(INVESTM_BUILD_TESTS "Build the GoogleTest unit tests" ON)
option(INVESTM_BUILD_EXAMPLES "Build the worked-example programs" ON)
option(INVESTM_BUILD_COMPARE
  "Build the QuantLib comparison benchmarks (needs QuantLib, Google Benchmark)"
  OFF)

include(cmake/investm.cmake)

if(INVESTM_BUILD_TESTS)
  enable_testing()
  # Prefer the distribution package (Ubuntu: libgtest-dev); otherwise
  # download GoogleTest at configure time.
  find_package(GTest 1.11 CONFIG QUIET)
  if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
      URL_HASH SHA256=b3e03aaaff81d68730d67392a135ac3fdfdf022880c66632ab188d2fe084cda3
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  endif()
  include(GoogleTest)
endif()

add_subdirectory(cpp)
