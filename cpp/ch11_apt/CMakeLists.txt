investm_library(investm_ch11 SRCS apt.cc DEPS investm_core)
investm_test(ch11_test SRCS apt_test.cc DEPS investm_ch11)
investm_example(ch11_examples SRCS examples_main.cc DEPS investm_ch11)
