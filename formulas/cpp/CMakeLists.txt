cmake_minimum_required(VERSION 3.20)
project(investm_formulas LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

add_library(investm_formulas
  src/derivatives.cc
  src/equity.cc
  src/factor_models.cc
  src/fixed_income.cc
  src/margin.cc
  src/performance.cc
  src/portfolio.cc
  src/regression.cc
)
target_include_directories(investm_formulas PUBLIC include)
target_compile_options(investm_formulas PRIVATE -Wall -Wextra -Wpedantic -Werror)

option(INVESTM_FORMULAS_BUILD_TESTS "Build the GoogleTest suite" ON)
if(INVESTM_FORMULAS_BUILD_TESTS)
  enable_testing()
  find_package(GTest QUIET)
  if(NOT GTest_FOUND)
    include(FetchContent)
    FetchContent_Declare(googletest
      URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
      DOWNLOAD_EXTRACT_TIMESTAMP TRUE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  endif()

  add_executable(investm_formulas_test
    tests/derivatives_test.cc
    tests/equity_test.cc
    tests/fixed_income_test.cc
    tests/margin_factor_test.cc
    tests/performance_test.cc
    tests/portfolio_test.cc
    tests/regression_test.cc
  )
  target_compile_options(investm_formulas_test PRIVATE -Wall -Wextra -Werror)
  target_link_libraries(investm_formulas_test
    PRIVATE investm_formulas GTest::gtest_main)
  include(GoogleTest)
  gtest_discover_tests(investm_formulas_test)
endif()
