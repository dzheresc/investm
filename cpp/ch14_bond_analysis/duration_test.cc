#include "cpp/ch14_bond_analysis/duration.h"

#include <cmath>

#include "gtest/gtest.h"

namespace investm::ch14 {
namespace {

using ch05::CouponBond;
using ch05::PresentValueAtYield;
using ch05::ZeroCoupon;

TEST(DurationTest, ZeroCouponDurationIsMaturity) {
  EXPECT_NEAR(MacaulayDuration(ZeroCoupon(100.0, 7.0), 0.05), 7.0, 1e-12);
  EXPECT_NEAR(ModifiedDuration(ZeroCoupon(100.0, 7.0), 0.05), 7.0 / 1.05,
              1e-12);
}

TEST(DurationTest, AnalyticDerivativesMatchFiniteDifferences) {
  const auto flows = CouponBond(100.0, 0.06, 10.0, 2);
  const double y = 0.07;
  auto price = [&](double r) { return PresentValueAtYield(flows, r, 2); };
  EXPECT_NEAR(ModifiedDuration(flows, y, 2), EffectiveDuration(price, y), 1e-5);
  EXPECT_NEAR(Convexity(flows, y, 2), EffectiveConvexity(price, y, 1e-3), 1e-3);
  EXPECT_NEAR(DollarDuration(flows, y, 2),
              ModifiedDuration(flows, y, 2) * price(y), 1e-10);
  EXPECT_NEAR(PriceValueOfBasisPoint(flows, y, 2),
              price(y - 0.00005) - price(y + 0.00005), 1e-6);
}

TEST(DurationTest, ConvexityImprovesTheEstimate) {
  const auto flows = CouponBond(100.0, 0.05, 20.0, 1);
  const double y = 0.05;
  const double p = PresentValueAtYield(flows, y);
  const double d = ModifiedDuration(flows, y);
  const double c = Convexity(flows, y);
  for (double dy : {-0.02, 0.02}) {
    const double actual = PresentValueAtYield(flows, y + dy) - p;
    const double first = ApproxPriceChange(p, d, 0.0, dy);
    const double second = ApproxPriceChange(p, d, c, dy);
    EXPECT_LT(std::abs(actual - second), std::abs(actual - first));
    EXPECT_GT(actual, first);  // Positive convexity: price above tangent.
  }
}

TEST(DurationTest, PricingTheorems) {
  // 1. Price and yield move inversely; 2-3. longer maturity -> larger
  // sensitivity, at a decreasing rate; 4. a fall in yield raises price by
  // more than an equal rise lowers it; 5. higher coupon -> smaller
  // percentage sensitivity.
  auto pct = [](double coupon, double years, double dy) {
    const auto f = CouponBond(100.0, coupon, years);
    const double p0 = PresentValueAtYield(f, 0.06);
    return PresentValueAtYield(f, 0.06 + dy) / p0 - 1.0;
  };
  EXPECT_LT(pct(0.06, 10, 0.01), 0.0);
  const double m5 = -pct(0.06, 5, 0.01);
  const double m10 = -pct(0.06, 10, 0.01);
  const double m15 = -pct(0.06, 15, 0.01);
  EXPECT_GT(m10, m5);
  EXPECT_LT(m15 - m10, m10 - m5);
  EXPECT_GT(pct(0.06, 10, -0.01), -pct(0.06, 10, 0.01));
  EXPECT_LT(-pct(0.10, 10, 0.01), -pct(0.02, 10, 0.01));
}

TEST(DurationTest, LongBondDurationApproachesPerpetuity) {
  const auto flows = CouponBond(100.0, 0.08, 400.0);
  EXPECT_NEAR(MacaulayDuration(flows, 0.08), PerpetuityDuration(0.08), 1e-6);
}

}  // namespace
}  // namespace investm::ch14
