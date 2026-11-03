investm_library(investm_ch07 SRCS efficient_set.cc market_model.cc
  DEPS investm_ch06)
investm_test(ch07_test SRCS efficient_set_test.cc market_model_test.cc
  DEPS investm_ch07)
investm_example(ch07_examples SRCS examples_main.cc DEPS investm_ch07)
