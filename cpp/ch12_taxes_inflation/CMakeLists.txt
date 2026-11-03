investm_library(investm_ch12 SRCS taxes.cc inflation.cc DEPS investm_ch05)
investm_test(ch12_test SRCS taxes_test.cc inflation_test.cc DEPS investm_ch12)
investm_example(ch12_examples SRCS examples_main.cc DEPS investm_ch12)
