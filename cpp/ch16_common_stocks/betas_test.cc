#include "cpp/ch16_common_stocks/betas.h"

#include <vector>

#include "gtest/gtest.h"

namespace investm::ch16 {
namespace {

TEST(BetasTest, EstimateAndRolling) {
  const std::vector<double> m{0.02, -0.01, 0.03, 0.00, -0.02, 0.04,
                              0.01, -0.03, 0.02, 0.05, -0.01, 0.00};
  std::vector<double> s;
  for (std::size_t t = 0; t < m.size(); ++t) {
    s.push_back(0.001 + (t < 6 ? 1.5 : 0.8) * m[t]);
  }
  const auto rolling = RollingBetas(s, m, 6);
  ASSERT_EQ(rolling.size(), 2u);
  EXPECT_NEAR(rolling[0], 1.5, 1e-12);
  EXPECT_NEAR(rolling[1], 0.8, 1e-12);
  const auto full = EstimateBeta(s, m);
  EXPECT_GT(full.beta, 0.8);
  EXPECT_LT(full.beta, 1.5);
}

TEST(BetasTest, Adjustments) {
  EXPECT_NEAR(ShrinkBeta(1.6), 1.4, 1e-15);
  EXPECT_NEAR(ShrinkBeta(0.4), 0.6, 1e-15);
  // Equal precision -> halfway; precise raw estimate -> close to raw.
  EXPECT_NEAR(VasicekBeta(1.6, 0.3, 1.0, 0.3), 1.3, 1e-15);
  EXPECT_NEAR(VasicekBeta(1.6, 0.01, 1.0, 0.5), 1.6, 1e-3);
  const std::vector<double> earlier{0.5, 0.8, 1.0, 1.3, 1.7};
  const std::vector<double> later{0.7, 0.9, 1.0, 1.2, 1.4};
  EXPECT_LT(BetaPersistence(earlier, later).beta, 1.0);
}

TEST(BetasTest, Leverage) {
  const double bl = LeveredBeta(0.9, 0.5, 0.25);
  EXPECT_NEAR(bl, 0.9 * 1.375, 1e-15);
  EXPECT_NEAR(UnleveredBeta(bl, 0.5, 0.25), 0.9, 1e-15);
  EXPECT_DOUBLE_EQ(LeveredBeta(1.1, 0.0, 0.3), 1.1);
}

}  // namespace
}  // namespace investm::ch16
