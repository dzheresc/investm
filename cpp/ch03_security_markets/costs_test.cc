#include "cpp/ch03_security_markets/costs.h"

#include "gtest/gtest.h"

namespace investm::ch03 {
namespace {

TEST(CostsTest, CommissionMinimumApplies) {
  const CommissionSchedule c{10.0, 0.01, 0.0, 15.0};
  EXPECT_DOUBLE_EQ(c.Commission(100, 50), 15.0);
  EXPECT_DOUBLE_EQ(c.Commission(1000, 50), 20.0);
}

TEST(CostsTest, SpreadMeasures) {
  const Quote q{9.95, 10.05};
  EXPECT_NEAR(q.RelativeSpread(), 0.01, 1e-12);
  EXPECT_NEAR(EffectiveHalfSpread(10.02, q), 0.002, 1e-12);
}

TEST(CostsTest, PriceImpactAndTotal) {
  EXPECT_NEAR(SqrtPriceImpact(10000, 1000000, 0.02), 0.002, 1e-15);
  const Quote q{9.95, 10.05};
  const CommissionSchedule c{0.0, 0.0, 0.001, 0.0};
  EXPECT_NEAR(OneWayCost(1000, 10.0, q, c, 0.002), 0.001 + 0.005 + 0.002,
              1e-12);
}

}  // namespace
}  // namespace investm::ch03
