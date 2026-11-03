#include <gtest/gtest.h>

#include "investm/formulas/factor_models.h"
#include "investm/formulas/margin.h"

namespace investm::formulas {
namespace {

TEST(MarginTest, RateOfReturn) {
  EXPECT_DOUBLE_EQ(RateOfReturn(100.0, 110.0), 0.1);
}

TEST(MarginTest, PurchaseMarginCallPriceHitsMaintenanceMargin) {
  EXPECT_DOUBLE_EQ(ActualMarginPurchase(100, 80.0, 0.6, 100.0), 0.5);
  const double call = MarginCallPricePurchase(0.6, 100.0, 0.3);
  EXPECT_DOUBLE_EQ(call, 40.0 / 0.7);
  EXPECT_NEAR(ActualMarginPurchase(100, call, 0.6, 100.0), 0.3, 1e-12);
}

TEST(MarginTest, ShortSaleMarginCallPriceHitsMaintenanceMargin) {
  EXPECT_DOUBLE_EQ(ActualMarginShortSale(100, 100.0, 0.5, 120.0), 0.25);
  const double call = MarginCallPriceShortSale(100.0, 0.5, 0.3);
  EXPECT_DOUBLE_EQ(call, 150.0 / 1.3);
  EXPECT_NEAR(ActualMarginShortSale(100, 100.0, 0.5, call), 0.3, 1e-12);
}

TEST(FactorModelTest, TwoFactorReturnAndVariance) {
  EXPECT_NEAR(TwoFactorReturn(0.01, 1.2, 0.05, 0.5, 0.02, 0.003), 0.083, 1e-12);
  EXPECT_NEAR(TwoFactorVariance(1.0, 2.0, 0.04, 0.01, 0.005, 0.02), 0.12,
              1e-12);
  EXPECT_NEAR(TwoFactorCovariance(1.0, 2.0, 1.0, 2.0, 0.04, 0.01, 0.005),
              TwoFactorVariance(1.0, 2.0, 0.04, 0.01, 0.005, 0.0), 1e-12);
}

TEST(FactorModelTest, AptFormsAgree) {
  const double rf = 0.04, d1 = 0.10, d2 = 0.07, b1 = 1.1, b2 = 0.6;
  EXPECT_NEAR(AptExpectedReturn(rf, d1 - rf, d2 - rf, b1, b2),
              AptExpectedReturnFromFactorPortfolios(rf, d1, d2, b1, b2), 1e-12);
}

}  // namespace
}  // namespace investm::formulas
