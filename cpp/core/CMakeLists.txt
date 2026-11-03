investm_library(investm_core
  SRCS matrix.cc regression.cc stats.cc table.cc)

investm_test(core_test
  SRCS matrix_test.cc root_find_test.cc stats_test.cc
  DEPS investm_core)
