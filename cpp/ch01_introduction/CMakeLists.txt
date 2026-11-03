investm_library(investm_ch01 SRCS returns.cc DEPS investm_core)
investm_test(ch01_test SRCS returns_test.cc DEPS investm_ch01)
investm_example(ch01_examples SRCS examples_main.cc DEPS investm_ch01)
