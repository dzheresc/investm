#include "cpp/ch06_portfolio_selection/risk_measures.h"

#include <vector>

#include "cpp/core/stats.h"
#include "gtest/gtest.h"

namespace investm::ch06 {
namespace {

TEST(RiskMeasuresTest, DownsideMeasures) {
  const std::vector<double> r{-0.10, -0.02, 0.03, 0.05, 0.08};
  EXPECT_NEAR(SemiVariance(r, 0.0), (0.01 + 0.0004) / 5.0, 1e-15);
  EXPECT_NEAR(ShortfallProbability(r, 0.0), 0.4, 1e-15);
}

TEST(RiskMeasuresTest, HistoricalVaRAndShortfall) {
  std::vector<double> r;
  for (int i = 1; i <= 100; ++i) r.push_back(i / 1000.0 - 0.05);  // -0.049..
  // Worst 5 of 100: -0.049, -0.048, ..., -0.045.
  EXPECT_NEAR(HistoricalVaR(r, 0.95), 0.045, 1e-12);
  EXPECT_NEAR(HistoricalExpectedShortfall(r, 0.95), 0.047, 1e-12);
}

TEST(RiskMeasuresTest, NormalQuantilesAndVaR) {
  EXPECT_NEAR(NormInv(0.975), 1.959963984540054, 1e-12);
  EXPECT_NEAR(NormInv(0.001), -3.090232306167814, 1e-11);
  EXPECT_NEAR(stats::NormCdf(NormInv(0.3)), 0.3, 1e-14);
  EXPECT_NEAR(NormalVaR(0.0, 0.1, 0.95), 0.1644853626951472, 1e-12);
  EXPECT_GT(NormalExpectedShortfall(0.0, 0.1, 0.95), NormalVaR(0.0, 0.1, 0.95));
}

}  // namespace
}  // namespace investm::ch06
