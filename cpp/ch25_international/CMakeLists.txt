investm_library(investm_ch25 SRCS international.cc DEPS investm_core)
investm_test(ch25_test SRCS international_test.cc DEPS investm_ch25)
investm_example(ch25_examples SRCS examples_main.cc DEPS investm_ch25)
