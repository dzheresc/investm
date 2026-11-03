investm_library(investm_ch23 SRCS management.cc DEPS investm_core)
investm_test(ch23_test SRCS management_test.cc DEPS investm_ch23)
investm_example(ch23_examples SRCS examples_main.cc DEPS investm_ch23
  investm_ch07)
