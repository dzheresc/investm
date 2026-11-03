investm_library(investm_ch05 SRCS rates.cc cash_flows.cc term_structure.cc
  DEPS investm_core)
investm_test(ch05_test
  SRCS rates_test.cc cash_flows_test.cc term_structure_test.cc
  DEPS investm_ch05)
investm_example(ch05_examples SRCS examples_main.cc DEPS investm_ch05)
