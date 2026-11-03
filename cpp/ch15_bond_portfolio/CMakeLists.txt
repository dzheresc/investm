investm_library(investm_ch15 SRCS bond_portfolio.cc DEPS investm_ch05)
investm_test(ch15_test SRCS bond_portfolio_test.cc DEPS investm_ch15
  investm_ch14)
investm_example(ch15_examples SRCS examples_main.cc DEPS investm_ch15
  investm_ch14)
