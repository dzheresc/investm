#include "cpp/ch12_taxes_inflation/taxes.h"

#include "gtest/gtest.h"

namespace investm::ch12 {
namespace {

TaxSchedule Schedule() {
  return TaxSchedule({{0.0, 0.10}, {20000.0, 0.20}, {80000.0, 0.35}});
}

TEST(TaxScheduleTest, ProgressiveTax) {
  const TaxSchedule s = Schedule();
  EXPECT_DOUBLE_EQ(s.Tax(0.0), 0.0);
  EXPECT_DOUBLE_EQ(s.Tax(10000.0), 1000.0);
  EXPECT_DOUBLE_EQ(s.Tax(50000.0), 2000.0 + 6000.0);
  EXPECT_DOUBLE_EQ(s.Tax(100000.0), 2000.0 + 12000.0 + 7000.0);
  EXPECT_DOUBLE_EQ(s.MarginalRate(50000.0), 0.20);
  EXPECT_DOUBLE_EQ(s.MarginalRate(100000.0), 0.35);
  EXPECT_DOUBLE_EQ(s.AverageRate(100000.0), 0.21);
  // Average rate is below the marginal rate under a progressive schedule.
  for (double y = 1000.0; y < 200000.0; y += 7000.0) {
    EXPECT_LE(s.AverageRate(y), s.MarginalRate(y) + 1e-15);
  }
}

TEST(TaxesTest, TaxableEquivalentYieldRoundTrips) {
  const double t = CombinedTaxRate(0.30, 0.05, true);
  EXPECT_NEAR(t, 0.335, 1e-15);
  EXPECT_NEAR(CombinedTaxRate(0.30, 0.05, false), 0.35, 1e-15);
  const double te = TaxableEquivalentYield(0.04, t);
  EXPECT_NEAR(te * (1.0 - t), 0.04, 1e-15);
  EXPECT_NEAR(BreakevenTaxRate(te, 0.04), t, 1e-14);
  EXPECT_NEAR(AfterTaxReturn(0.10, 0.25), 0.075, 1e-15);
}

TEST(TaxesTest, DeferralBeatsAnnualTaxation) {
  const double a = WealthTaxedAnnually(1000.0, 0.08, 0.30, 20.0);
  const double e = WealthTaxedAtEnd(1000.0, 0.08, 0.30, 20.0);
  const double u = WealthUntaxed(1000.0, 0.08, 20.0);
  EXPECT_LT(a, e);
  EXPECT_LT(e, u);
  // One period: deferral makes no difference.
  EXPECT_NEAR(WealthTaxedAnnually(1000.0, 0.08, 0.30, 1.0),
              WealthTaxedAtEnd(1000.0, 0.08, 0.30, 1.0), 1e-12);
  const double g = EffectiveDeferredRate(0.08, 0.30, 20.0);
  EXPECT_GT(g, 0.08 * 0.70);
  EXPECT_LT(g, 0.08);
  EXPECT_NEAR(WealthUntaxed(1000.0, g, 20.0), e, 1e-9);
}

}  // namespace
}  // namespace investm::ch12
