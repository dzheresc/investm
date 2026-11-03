#include "cpp/ch21_investment_companies/funds.h"

#include <cmath>
#include <vector>

#include "gtest/gtest.h"

namespace investm::ch21 {
namespace {

TEST(FundsTest, NavAndPremium) {
  EXPECT_DOUBLE_EQ(NetAssetValue(520e6, 20e6, 25e6), 20.0);
  EXPECT_NEAR(PremiumOrDiscount(18.0, 20.0), -0.10, 1e-15);
  EXPECT_NEAR(PremiumOrDiscount(21.0, 20.0), 0.05, 1e-15);
}

TEST(FundsTest, Loads) {
  EXPECT_NEAR(OfferingPrice(19.0, 0.05), 20.0, 1e-12);
  EXPECT_NEAR(LoadAsFractionOfInvestment(0.05), 0.05 / 0.95, 1e-15);
}

TEST(FundsTest, ReturnsWithDistributions) {
  EXPECT_NEAR(FundReturn(20.0, 21.0, 0.40, 0.60), 0.10, 1e-15);
  // Reinvesting distributions: value equals (1 + return) x initial value.
  const double shares = SharesAfterReinvestment(100.0, 1.0, 21.0);
  EXPECT_NEAR(shares * 21.0, 100.0 * 20.0 * 1.10, 1e-10);
}

TEST(FundsTest, TerminalValueAndDrag) {
  EXPECT_NEAR(TerminalValue(1000.0, 0.08, 0.0, 10, 0.0),
              1000.0 * std::pow(1.08, 10), 1e-9);
  const double v = TerminalValue(1000.0, 0.08, 0.01, 10, 0.05);
  EXPECT_NEAR(v, 950.0 * std::pow(1.08 * 0.99, 10), 1e-9);
  const std::vector<double> cdsc{0.05, 0.04, 0.03, 0.02, 0.01};
  EXPECT_NEAR(TerminalValue(1000.0, 0.0, 0.0, 2, 0.0, cdsc), 960.0, 1e-12);
  EXPECT_NEAR(TerminalValue(1000.0, 0.0, 0.0, 6, 0.0, cdsc), 1000.0, 1e-12);
  // Front-load drag shrinks with the holding period.
  EXPECT_GT(AnnualizedFeeDrag(0.08, 0.005, 1, 0.05),
            AnnualizedFeeDrag(0.08, 0.005, 20, 0.05));
  EXPECT_NEAR(AnnualizedFeeDrag(0.08, 0.0, 5, 0.0), 0.0, 1e-14);
}

TEST(FundsTest, BreakevenHorizon) {
  const FundTerms load{0.004, 0.0475};
  const FundTerms no_load{0.012, 0.0};
  const auto y = BreakevenYears(load, no_load, 0.08);
  ASSERT_TRUE(y.has_value());
  EXPECT_GT(TerminalValue(1.0, 0.08, 0.004, *y, 0.0475),
            TerminalValue(1.0, 0.08, 0.012, *y, 0.0) - 1e-15);
  EXPECT_LT(TerminalValue(1.0, 0.08, 0.004, *y - 1, 0.0475),
            TerminalValue(1.0, 0.08, 0.012, *y - 1, 0.0));
  EXPECT_FALSE(BreakevenYears({0.012, 0.05}, {0.004, 0.0}, 0.08).has_value());
}

}  // namespace
}  // namespace investm::ch21
