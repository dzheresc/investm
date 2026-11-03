#include "cpp/ch01_introduction/returns.h"

#include <cmath>
#include <vector>

#include "gtest/gtest.h"

namespace investm::ch01 {
namespace {

TEST(ReturnsTest, HoldingPeriodReturnIncludesIncome) {
  EXPECT_NEAR(HoldingPeriodReturn(40.0, 43.0, 1.2), 0.105, 1e-15);
}

TEST(ReturnsTest, SimpleAndCumulativeReturnsAreConsistent) {
  const std::vector<double> prices{100, 110, 99};
  const std::vector<double> r = SimpleReturns(prices);
  ASSERT_EQ(r.size(), 2u);
  EXPECT_NEAR(r[0], 0.10, 1e-15);
  EXPECT_NEAR(r[1], -0.10, 1e-15);
  EXPECT_NEAR(CumulativeReturn(r), -0.01, 1e-15);
  EXPECT_NEAR(Annualize(0.21, 2.0), 0.10, 1e-12);
}

TEST(ReturnsTest, IncomeIsAddedToEachPeriod) {
  const std::vector<double> prices{50, 50};
  const std::vector<double> income{1};
  EXPECT_NEAR(SimpleReturns(prices, income)[0], 0.02, 1e-15);
}

TEST(ReturnsTest, DiversificationLimits) {
  EXPECT_NEAR(EqualWeightPortfolioStdDev(1, 0.4, 0.25), 0.4, 1e-15);
  // Uncorrelated assets: sigma / sqrt(N).
  EXPECT_NEAR(EqualWeightPortfolioStdDev(100, 0.4, 0.0), 0.04, 1e-15);
  // Perfect correlation: no benefit at all.
  EXPECT_NEAR(EqualWeightPortfolioStdDev(50, 0.4, 1.0), 0.4, 1e-15);
  // Floor is sqrt(rho) * sigma = 0.2.
  EXPECT_NEAR(EqualWeightPortfolioStdDev(1000000, 0.4, 0.25), 0.2, 1e-6);
}

TEST(ReturnsDeathTest, NonPositiveBeginValueAborts) {
  EXPECT_DEATH(HoldingPeriodReturn(0.0, 1.0), "begin value");
}

}  // namespace
}  // namespace investm::ch01
