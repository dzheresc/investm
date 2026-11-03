investm_library(investm_ch22 SRCS ratios.cc technical.cc DEPS investm_core)
investm_test(ch22_test SRCS ratios_test.cc technical_test.cc
  DEPS investm_ch22)
investm_example(ch22_examples SRCS examples_main.cc DEPS investm_ch22)
