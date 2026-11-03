investm_library(investm_ch06 SRCS utility.cc portfolio.cc risk_measures.cc
  DEPS investm_core)
investm_test(ch06_test
  SRCS utility_test.cc portfolio_test.cc risk_measures_test.cc
  DEPS investm_ch06)
investm_example(ch06_examples SRCS examples_main.cc DEPS investm_ch06)
