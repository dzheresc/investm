# Comparison benchmarks against QuantLib; see docs/comparison.md.
# Ubuntu: sudo apt-get install libquantlib0-dev libbenchmark-dev
find_package(benchmark CONFIG REQUIRED)

# A QuantLib built from source installs a CMake package; Ubuntu's
# libquantlib0-dev installs only a pkg-config file.
find_package(QuantLib CONFIG QUIET)
if(TARGET QuantLib::QuantLib)
  set(INVESTM_QUANTLIB QuantLib::QuantLib)
else()
  find_package(PkgConfig REQUIRED)
  pkg_check_modules(QUANTLIB REQUIRED IMPORTED_TARGET quantlib)
  set(INVESTM_QUANTLIB PkgConfig::QUANTLIB)
endif()

add_executable(investm_compare_quantlib compare_quantlib.cc)
target_include_directories(investm_compare_quantlib PRIVATE ${PROJECT_SOURCE_DIR})
target_link_libraries(investm_compare_quantlib PRIVATE investm_core
  investm_ch05 investm_ch06 investm_ch13 investm_ch14 investm_ch19
  ${INVESTM_QUANTLIB} benchmark::benchmark_main)
investm_set_warnings(investm_compare_quantlib)
