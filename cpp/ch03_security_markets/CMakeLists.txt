investm_library(investm_ch03 SRCS call_market.cc costs.cc DEPS investm_core)
investm_test(ch03_test SRCS call_market_test.cc costs_test.cc
  DEPS investm_ch03)
investm_example(ch03_examples SRCS examples_main.cc DEPS investm_ch03)
