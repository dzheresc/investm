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

// AS241 must hold full precision in both tails and the central region with
// no refinement. In the tail a relative error e in z moves N(z) by about
// z^2 e relatively, so the round trip is checked to 4e-15 (1 + z^2).
TEST(RiskMeasuresTest, NormInvIsAccurateAcrossTheRange) {
  for (double p = 1e-300; p < 0.5; p *= 1.7) {
    const double z = NormInv(p);
    EXPECT_NEAR(stats::NormCdf(z) / p, 1.0, 4e-15 * (1.0 + z * z))
        << "p = " << p;
  }
  for (int i = 1; i < 1000; ++i) {
    const double p = i / 1000.0;
    EXPECT_NEAR(stats::NormCdf(NormInv(p)), p, 4e-16) << "p = " << p;
  }
  EXPECT_EQ(NormInv(0.5), 0.0);
  EXPECT_DOUBLE_EQ(NormInv(0.975), -NormInv(0.025));
}

}  // namespace
}  // namespace investm::ch06
