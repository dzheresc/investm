investm_library(investm_ch04 SRCS schedules.cc efficiency_tests.cc
  DEPS investm_core)
investm_test(ch04_test SRCS schedules_test.cc efficiency_tests_test.cc
  DEPS investm_ch04)
investm_example(ch04_examples SRCS examples_main.cc DEPS investm_ch04)
