#include "investm/formulas/equity.h"

#include <gtest/gtest.h>

#include <cmath>
#include <vector>

namespace investm::formulas {
namespace {

TEST(EquityTest, VotingAndRights) {
  EXPECT_DOUBLE_EQ(SharesToElectDirectors(2, 1000, 4), 401.0);
  EXPECT_DOUBLE_EQ(RightValueRightsOn(60.0, 50.0, 4), 2.0);
  EXPECT_DOUBLE_EQ(RightValueExRights(58.0, 50.0, 4), 2.0);
}

TEST(EquityTest, FirmAndEquityBetaRoundTrip) {
  const double tax = 0.3, debt = 40.0, equity = 60.0;
  const double beta_debt = 0.2, beta_firm = 0.9;
  const double unlevered = equity + (1.0 - tax) * debt;
  const double beta_equity =
      EquityBeta(beta_firm, beta_debt, debt, equity, tax);
  EXPECT_NEAR(FirmBeta(beta_debt, beta_equity, tax, debt, equity, unlevered),
              beta_firm, 1e-12);
  EXPECT_NEAR(AdjustedBeta(0.33, 0.67, 1.5), 1.335, 1e-12);
}

TEST(EquityTest, DividendDiscountModels) {
  EXPECT_DOUBLE_EQ(ZeroGrowthDdm(2.0, 0.1), 20.0);
  EXPECT_NEAR(ConstantGrowthDdm(2.0, 0.1, 0.05), 40.0, 1e-10);
  EXPECT_TRUE(std::isnan(ConstantGrowthDdm(2.0, 0.05, 0.05)));
  EXPECT_NEAR(MultipleGrowthDdm({}, 2.0, 0.1, 0.05), 40.0, 1e-10);
  const std::vector<double> divs = {1.0, 1.5};
  EXPECT_NEAR(MultipleGrowthDdm(divs, 1.575, 0.1, 0.05),
              1 / 1.1 + 1.5 / 1.21 + 1.575 / (0.05 * 1.21), 1e-10);
  EXPECT_NEAR(CapitalizedValue(std::vector<double>{1.0, 1.0}, 0.1),
              1 / 1.1 + 1 / 1.21, 1e-12);
}

TEST(EquityTest, NormalPriceEarnings) {
  EXPECT_NEAR(NormalPriceEarnings(1.0, 0.15, 0.1), 10.0, 1e-10);
  const double g = 0.15 * 0.4;
  EXPECT_NEAR(NormalPriceEarnings(0.6, 0.15, 0.1), 0.6 * (1 + g) / (0.1 - g),
              1e-10);
}

TEST(EquityTest, EarningsModels) {
  EXPECT_NEAR(LintnerDividend(0.5, 0.4, 10.0, 3.0), 3.5, 1e-12);
  EXPECT_NEAR(RandomWalkEarnings(5.0, -0.5), 4.5, 1e-12);
  EXPECT_NEAR(QuarterlyEarnings(1.0, 1.2, 0.9, 0.4, 0.05, 0.01), 1.18, 1e-12);
}

}  // namespace
}  // namespace investm::formulas
