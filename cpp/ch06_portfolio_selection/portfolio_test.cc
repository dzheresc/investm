#include "cpp/ch06_portfolio_selection/portfolio.h"

#include <cmath>
#include <vector>

#include "gtest/gtest.h"

namespace investm::ch06 {
namespace {

TEST(PortfolioTest, WeightsAndReturnFromHoldings) {
  const std::vector<double> shares{150, 300, 80};
  const std::vector<double> prices{30, 22, 75};
  const auto w = WeightsFromHoldings(shares, prices);
  EXPECT_NEAR(w[0] + w[1] + w[2], 1.0, 1e-15);
  EXPECT_NEAR(w[0], 4500.0 / 17100.0, 1e-15);
  const std::vector<double> end{33, 21, 84};
  const double r = PortfolioReturnFromPrices(shares, prices, end);
  EXPECT_NEAR(r, (4950.0 + 6300.0 + 6720.0) / 17100.0 - 1.0, 1e-15);
}

TEST(PortfolioTest, MomentsMatchTwoAssetFormula) {
  const Matrix cov({{0.04, 0.3 * 0.2 * 0.3}, {0.3 * 0.2 * 0.3, 0.09}});
  const std::vector<double> w{0.6, 0.4};
  EXPECT_NEAR(PortfolioStdDev(w, cov), TwoAssetStdDev(0.6, 0.2, 0.3, 0.3),
              1e-15);
  EXPECT_NEAR(PortfolioExpectedReturn(w, std::vector<double>{0.1, 0.2}), 0.14,
              1e-15);
  // Perfect negative correlation allows a riskless combination.
  EXPECT_NEAR(TwoAssetStdDev(0.6, 0.2, 0.3, -1.0), 0.0, 1e-15);
}

TEST(PortfolioTest, ScenarioMomentsAndPortfolioReturns) {
  // Three equally likely states, two securities.
  const Matrix r({{0.10, -0.05}, {0.05, 0.05}, {0.00, 0.15}});
  const std::vector<double> p{1.0 / 3, 1.0 / 3, 1.0 / 3};
  const auto m = MomentsFromScenarios(r, p);
  EXPECT_NEAR(m.expected[0], 0.05, 1e-15);
  EXPECT_NEAR(m.expected[1], 0.05, 1e-15);
  EXPECT_NEAR(m.cov(0, 1), (0.05 * -0.10 + 0.0 + -0.05 * 0.10) / 3.0, 1e-15);
  const std::vector<double> w{0.5, 0.5};
  const auto rp = ScenarioPortfolioReturns(r, w);
  EXPECT_NEAR(rp[0], 0.025, 1e-15);
  // Variance from scenarios equals w' C w.
  double var = 0.0;
  for (double x : rp) var += (x - 0.05) * (x - 0.05) / 3.0;
  EXPECT_NEAR(var, PortfolioVariance(w, m.cov), 1e-15);
}

}  // namespace
}  // namespace investm::ch06
