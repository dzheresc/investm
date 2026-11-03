investm_library(investm_ch17 SRCS ddm.cc DEPS investm_core)
investm_test(ch17_test SRCS ddm_test.cc DEPS investm_ch17)
investm_example(ch17_examples SRCS examples_main.cc DEPS investm_ch17)
