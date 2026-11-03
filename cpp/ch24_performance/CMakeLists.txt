investm_library(investm_ch24 SRCS performance.cc DEPS investm_ch05
  investm_ch07)
investm_test(ch24_test SRCS performance_test.cc DEPS investm_ch24)
investm_example(ch24_examples SRCS examples_main.cc DEPS investm_ch24)
