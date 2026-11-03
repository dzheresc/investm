investm_library(investm_ch13 SRCS fixed_income.cc DEPS investm_ch05)
investm_test(ch13_test SRCS fixed_income_test.cc DEPS investm_ch13)
investm_example(ch13_examples SRCS examples_main.cc DEPS investm_ch13)
