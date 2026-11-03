investm_library(investm_ch18 SRCS earnings.cc DEPS investm_core)
investm_test(ch18_test SRCS earnings_test.cc DEPS investm_ch18)
investm_example(ch18_examples SRCS examples_main.cc DEPS investm_ch18)
