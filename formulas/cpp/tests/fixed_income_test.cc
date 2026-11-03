#include "investm/formulas/fixed_income.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

namespace investm::formulas {
namespace {

TEST(FixedIncomeTest, TaxGrowthAndRealReturn) {
  EXPECT_NEAR(BreakEvenTaxRate(0.06, 0.08), 0.25, 1e-12);
  EXPECT_NEAR(GeometricMeanGrowthRate(100.0, 121.0, 2.0), 0.1, 1e-12);
  EXPECT_NEAR(RealReturn(0.10, 0.04), 1.10 / 1.04 - 1.0, 1e-12);
  EXPECT_NEAR(ApproxRealReturn(0.10, 0.04), 0.06, 1e-12);
}

TEST(FixedIncomeTest, ParBondAndYieldRoundTrip) {
  const std::vector<double> coupons(10, 5.0);
  EXPECT_NEAR(BondPrice(coupons, 100.0, 0.05), 100.0, 1e-10);
  const double price = BondPrice(coupons, 100.0, 0.07);
  EXPECT_NEAR(YieldToMaturity(price, coupons, 100.0), 0.07, 1e-9);
  EXPECT_TRUE(std::isnan(YieldToMaturity(price, coupons, 100.0, 0.2, 0.3)));
}

TEST(FixedIncomeTest, SpotAndForwardRates) {
  EXPECT_NEAR(DiscountFactor(0.05, 2.0), 1.0 / (1.05 * 1.05), 1e-12);
  const double fwd = ForwardRate(0.05, 0.06, 2);
  EXPECT_NEAR(1.05 * (1.0 + fwd), 1.06 * 1.06, 1e-12);
}

TEST(FixedIncomeTest, Durations) {
  EXPECT_NEAR(MacaulayDuration(std::vector<double>{0.0, 0.0, 100.0}, 0.08), 3.0,
              1e-12);
  const double d = MacaulayDuration(std::vector<double>{10.0, 110.0}, 0.10);
  EXPECT_NEAR(d, (10 / 1.1 + 2 * 110 / 1.21) / 100.0, 1e-12);
  const double dm = ModifiedDuration(d, 0.10);
  EXPECT_NEAR(PriceChangeFromDuration(d, 0.10, 0.001),
              PriceChangeFromModifiedDuration(dm, 0.001), 1e-12);
}

TEST(FixedIncomeTest, EffectiveDurationApproximatesModified) {
  const std::vector<double> coupons(10, 6.0);
  const double y = 0.06, dy = 1e-4;
  const double p = BondPrice(coupons, 100.0, y);
  const double up = BondPrice(coupons, 100.0, y + dy);
  const double down = BondPrice(coupons, 100.0, y - dy);
  std::vector<double> cash_flows = coupons;
  cash_flows.back() += 100.0;
  const double dm = ModifiedDuration(MacaulayDuration(cash_flows, y), y);
  EXPECT_NEAR(EffectiveDuration(down, up, p, dy), dm, 1e-5);
}

}  // namespace
}  // namespace investm::formulas
