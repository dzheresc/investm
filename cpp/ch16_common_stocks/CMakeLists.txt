investm_library(investm_ch16 SRCS corporate.cc betas.cc DEPS investm_core)
investm_test(ch16_test SRCS corporate_test.cc betas_test.cc DEPS investm_ch16)
investm_example(ch16_examples SRCS examples_main.cc DEPS investm_ch16)
