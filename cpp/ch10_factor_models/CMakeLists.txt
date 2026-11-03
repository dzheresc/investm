investm_library(investm_ch10 SRCS factor_models.cc DEPS investm_core)
investm_test(ch10_test SRCS factor_models_test.cc DEPS investm_ch10)
investm_example(ch10_examples SRCS examples_main.cc DEPS investm_ch10)
