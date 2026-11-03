investm_library(investm_ch19 SRCS payoffs.cc pricing.cc DEPS investm_core)
investm_test(ch19_test SRCS payoffs_test.cc pricing_test.cc DEPS investm_ch19)
investm_example(ch19_examples SRCS examples_main.cc DEPS investm_ch19)
