#include "cpp/ch19_options/payoffs.h"

#include <vector>

#include "gtest/gtest.h"

namespace investm::ch19 {
namespace {

TEST(PayoffTest, CallsAndPuts) {
  EXPECT_DOUBLE_EQ(Payoff(OptionType::kCall, 50.0, 58.0), 8.0);
  EXPECT_DOUBLE_EQ(Payoff(OptionType::kCall, 50.0, 45.0), 0.0);
  EXPECT_DOUBLE_EQ(Payoff(OptionType::kPut, 50.0, 45.0), 5.0);
  EXPECT_DOUBLE_EQ(Payoff(OptionType::kPut, 50.0, 58.0), 0.0);
}

TEST(PayoffTest, StraddleAndCoveredCall) {
  const std::vector<Leg> straddle{{Instrument::kCall, 100.0, 1.0, 6.0},
                                  {Instrument::kPut, 100.0, 1.0, 5.0}};
  EXPECT_DOUBLE_EQ(StrategyProfit(straddle, 100.0), -11.0);
  EXPECT_DOUBLE_EQ(StrategyProfit(straddle, 89.0), 0.0);
  EXPECT_DOUBLE_EQ(StrategyProfit(straddle, 111.0), 0.0);
  const std::vector<Leg> covered{{Instrument::kStock, 0.0, 1.0, 95.0},
                                 {Instrument::kCall, 100.0, -1.0, 4.0}};
  EXPECT_DOUBLE_EQ(StrategyPayoff(covered, 130.0), 100.0);
  EXPECT_DOUBLE_EQ(StrategyProfit(covered, 80.0), -11.0);
}

TEST(PayoffTest, ParityPortfoliosHaveEqualPayoffs) {
  // Call + bond paying K == put + stock, at every terminal price.
  const std::vector<Leg> fiduciary{{Instrument::kCall, 40.0, 1.0, 0.0},
                                   {Instrument::kBond, 40.0, 1.0, 0.0}};
  const std::vector<Leg> protective{{Instrument::kPut, 40.0, 1.0, 0.0},
                                    {Instrument::kStock, 0.0, 1.0, 0.0}};
  for (double s = 20.0; s <= 60.0; s += 5.0) {
    EXPECT_DOUBLE_EQ(StrategyPayoff(fiduciary, s),
                     StrategyPayoff(protective, s));
  }
  const double put = ParityPutPrice(5.0, 42.0, 40.0, 0.05, 0.5);
  EXPECT_NEAR(ParityCallPrice(put, 42.0, 40.0, 0.05, 0.5), 5.0, 1e-14);
  EXPECT_NEAR(ParityDeviation(5.0, put, 42.0, 40.0, 0.05, 0.5), 0.0, 1e-14);
  EXPECT_GT(CallLowerBound(42.0, 40.0, 0.05, 0.5), 2.0);
  EXPECT_DOUBLE_EQ(PutLowerBound(42.0, 40.0, 0.05, 0.5), 0.0);
}

}  // namespace
}  // namespace investm::ch19
