investm_library(investm_ch20 SRCS futures.cc DEPS investm_ch19)
investm_test(ch20_test SRCS futures_test.cc DEPS investm_ch20)
investm_example(ch20_examples SRCS examples_main.cc DEPS investm_ch20)
