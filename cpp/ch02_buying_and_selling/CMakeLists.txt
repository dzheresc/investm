investm_library(investm_ch02 SRCS orders.cc margin.cc DEPS investm_core)
investm_test(ch02_test SRCS orders_test.cc margin_test.cc DEPS investm_ch02)
investm_example(ch02_examples SRCS examples_main.cc DEPS investm_ch02)
