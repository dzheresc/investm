#include "cpp/ch05_riskless_securities/rates.h"

#include <cmath>

#include "gtest/gtest.h"

namespace investm::ch05 {
namespace {

TEST(RatesTest, FisherRoundTrip) {
  EXPECT_NEAR(RealRate(0.07, 0.03), 1.07 / 1.03 - 1.0, 1e-15);
  EXPECT_NEAR(NominalRate(RealRate(0.07, 0.03), 0.03), 0.07, 1e-15);
}

TEST(RatesTest, CompoundingConversions) {
  EXPECT_NEAR(EffectiveAnnualRate(0.08, 4), std::pow(1.02, 4) - 1.0, 1e-15);
  EXPECT_NEAR(AprFromEffective(EffectiveAnnualRate(0.08, 12), 12), 0.08, 1e-14);
  EXPECT_NEAR(ConvertCompounding(0.08, 2, 2), 0.08, 1e-14);
  EXPECT_NEAR(EffectiveAnnualRate(0.08, 1000000), EffectiveFromContinuous(0.08),
              1e-7);
  EXPECT_NEAR(ContinuousFromEffective(EffectiveFromContinuous(0.05)), 0.05,
              1e-15);
  EXPECT_NEAR(FutureValue(100, 0.06, 2, 3), 100 * std::pow(1.03, 6), 1e-12);
  EXPECT_NEAR(FutureValueContinuous(100, 0.05, 2), 100 * std::exp(0.1), 1e-12);
}

TEST(RatesTest, BankDiscountYields) {
  const double price = BillPriceFromDiscount(10000, 0.06, 90);
  EXPECT_NEAR(price, 9850.0, 1e-9);
  EXPECT_NEAR(BankDiscountYield(10000, price, 90), 0.06, 1e-15);
  EXPECT_NEAR(BondEquivalentYield(10000, price, 90),
              150.0 / 9850.0 * 365.0 / 90.0, 1e-15);
  EXPECT_NEAR(MoneyMarketYield(10000, price, 90), 150.0 / 9850.0 * 360.0 / 90.0,
              1e-15);
  EXPECT_GT(BondEquivalentYield(10000, price, 90),
            BankDiscountYield(10000, price, 90));
  EXPECT_GT(BillEffectiveAnnualYield(10000, price, 90),
            BondEquivalentYield(10000, price, 90));
}

}  // namespace
}  // namespace investm::ch05
