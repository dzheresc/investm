investm_library(investm_ch21 SRCS funds.cc DEPS investm_core)
investm_test(ch21_test SRCS funds_test.cc DEPS investm_ch21)
investm_example(ch21_examples SRCS examples_main.cc DEPS investm_ch21)
