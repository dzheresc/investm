investm_library(investm_ch08 SRCS riskfree.cc DEPS investm_ch07)
investm_test(ch08_test SRCS riskfree_test.cc DEPS investm_ch08)
investm_example(ch08_examples SRCS examples_main.cc DEPS investm_ch08)
