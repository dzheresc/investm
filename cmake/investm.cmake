# Helper functions shared by every chapter's CMakeLists.txt.
#
#   investm_library(<name> SRCS a.cc b.cc DEPS investm_core ...)
#   investm_test(<name> SRCS a_test.cc DEPS investm_ch05)
#   investm_example(<name> SRCS examples_main.cc DEPS investm_ch05)
#
# Includes are rooted at the repository root, Google style:
#   #include "cpp/ch05_riskless_securities/term_structure.h"

function(investm_set_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4
      $<$<BOOL:${INVESTM_WARNINGS_AS_ERRORS}>:/WX>)
  else()
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wshadow
      -Wconversion -Wsign-conversion
      $<$<BOOL:${INVESTM_WARNINGS_AS_ERRORS}>:-Werror>)
  endif()
endfunction()

function(investm_library name)
  cmake_parse_arguments(ARG "" "" "SRCS;DEPS" ${ARGN})
  add_library(${name} STATIC ${ARG_SRCS})
  target_include_directories(${name} PUBLIC ${PROJECT_SOURCE_DIR})
  target_link_libraries(${name} PUBLIC ${ARG_DEPS})
  investm_set_warnings(${name})
endfunction()

function(investm_test name)
  if(NOT INVESTM_BUILD_TESTS)
    return()
  endif()
  cmake_parse_arguments(ARG "" "" "SRCS;DEPS" ${ARGN})
  add_executable(${name} ${ARG_SRCS})
  target_link_libraries(${name} PRIVATE ${ARG_DEPS} GTest::gtest_main)
  investm_set_warnings(${name})
  gtest_discover_tests(${name} DISCOVERY_MODE PRE_TEST)
endfunction()

function(investm_example name)
  if(NOT INVESTM_BUILD_EXAMPLES)
    return()
  endif()
  cmake_parse_arguments(ARG "" "" "SRCS;DEPS" ${ARGN})
  add_executable(${name} ${ARG_SRCS})
  target_link_libraries(${name} PRIVATE ${ARG_DEPS})
  investm_set_warnings(${name})
  if(INVESTM_BUILD_TESTS)
    # Every worked example must at least run to completion.
    add_test(NAME ${name}_runs COMMAND ${name})
  endif()
endfunction()
