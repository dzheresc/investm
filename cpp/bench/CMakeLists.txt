# Micro-benchmarks for the performance and profiling work (see
# docs/performance.md and scripts/profile.sh).
add_executable(investm_bench bench.cc benchmarks.cc)
target_include_directories(investm_bench PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(investm_bench PRIVATE investm_core investm_ch05
  investm_ch07 investm_ch08 investm_ch10 investm_ch13 investm_ch14
  investm_ch19 investm_ch23 investm_ch24)
investm_set_warnings(investm_bench)
if(INVESTM_BUILD_TESTS)
  add_test(NAME investm_bench_smoke COMMAND investm_bench --quick)
endif()
