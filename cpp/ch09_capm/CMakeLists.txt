investm_library(investm_ch09 SRCS capm.cc DEPS investm_ch08)
investm_test(ch09_test SRCS capm_test.cc DEPS investm_ch09)
investm_example(ch09_examples SRCS examples_main.cc DEPS investm_ch09)
