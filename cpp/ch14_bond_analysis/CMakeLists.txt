investm_library(investm_ch14 SRCS duration.cc credit.cc DEPS investm_ch05)
investm_test(ch14_test SRCS duration_test.cc credit_test.cc DEPS investm_ch14)
investm_example(ch14_examples SRCS examples_main.cc DEPS investm_ch14)
