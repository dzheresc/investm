#include "cpp/ch22_financial_analysis/technical.h"

#include <cmath>
#include <vector>

#include "gtest/gtest.h"

namespace investm::ch22 {
namespace {

TEST(TechnicalTest, MovingAverageAndSignal) {
  const std::vector<double> p{10, 11, 12, 11, 10, 9, 10, 12};
  const auto ma = SimpleMovingAverage(p, 3);
  EXPECT_TRUE(std::isnan(ma[0]) && std::isnan(ma[1]));
  EXPECT_NEAR(ma[2], 11.0, 1e-15);
  EXPECT_NEAR(ma[5], 10.0, 1e-15);
  const auto s = MovingAverageSignal(p, 3);
  EXPECT_EQ(s, (std::vector<int>{0, 0, 1, -1, -1, -1, 1, 1}));
}

TEST(TechnicalTest, MomentumAndRelativeStrength) {
  const std::vector<double> p{100, 105, 110, 99};
  const auto m = Momentum(p, 2);
  EXPECT_NEAR(m[2], 0.10, 1e-15);
  EXPECT_NEAR(m[3], 99.0 / 105.0 - 1.0, 1e-15);
  const std::vector<double> idx{1000, 1050, 1050, 1100};
  const auto rs = RelativeStrength(p, idx);
  EXPECT_DOUBLE_EQ(rs[0], 1.0);
  EXPECT_NEAR(rs[1], 1.0, 1e-15);
  EXPECT_NEAR(rs[2], 1.1 / 1.05, 1e-15);
}

TEST(TechnicalTest, RuleBacktest) {
  const std::vector<double> p{10, 11, 12, 11, 10, 9, 10, 12};
  const auto r = MovingAverageRuleReturns(p, 3, 0.001);
  ASSERT_EQ(r.size(), 7u);
  EXPECT_NEAR(r[0], 0.001, 1e-15);            // No signal yet.
  EXPECT_NEAR(r[2], 11.0 / 12.0 - 1, 1e-15);  // Invested after t = 2.
  EXPECT_NEAR(r[4], 0.001, 1e-15);            // Out of the market.
}

}  // namespace
}  // namespace investm::ch22
