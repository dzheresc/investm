#include "cpp/ch18_earnings/earnings.h"

#include <vector>

#include "gtest/gtest.h"

namespace investm::ch18 {
namespace {

TEST(LintnerTest, RecoversParameters) {
  const LintnerModel truth{0.02, 0.35, 0.50, 1.0};
  const std::vector<double> e{2.0, 2.4, 2.1, 2.9, 3.3, 3.0, 3.8, 4.1};
  std::vector<double> d{0.9};
  for (std::size_t t = 1; t < e.size(); ++t) {
    d.push_back(truth.NextDividend(d.back(), e[t]));
  }
  const auto fit = FitLintner(e, d);
  ASSERT_TRUE(fit.has_value());
  EXPECT_NEAR(fit->a, 0.02, 1e-10);
  EXPECT_NEAR(fit->adjustment, 0.35, 1e-10);
  EXPECT_NEAR(fit->target_payout, 0.50, 1e-10);
  EXPECT_NEAR(fit->r_squared, 1.0, 1e-10);
}

TEST(ForecastTest, RandomWalks) {
  const std::vector<double> annual{1.0, 1.2, 1.1, 1.6};
  EXPECT_NEAR(RandomWalkForecast(annual), 1.8, 1e-15);
  // Seasonal pattern + 0.04 growth per year in every quarter.
  const std::vector<double> q{0.5, 0.8, 0.6, 1.0, 0.54, 0.84, 0.64, 1.04};
  EXPECT_NEAR(SeasonalRandomWalkForecast(q), 0.58, 1e-12);
}

TEST(ForecastTest, StandardizedUnexpectedEarnings) {
  const std::vector<double> q{0.50, 0.80, 0.60, 1.00, 0.53,
                              0.85, 0.63, 1.03, 0.58, 0.87};
  // Seasonal differences: .03 .05 .03 .03 .05 .02.
  const double forecast = SeasonalRandomWalkForecast(q);
  EXPECT_NEAR(forecast, 0.63 + (0.03 + 0.05 + 0.03 + 0.03 + 0.05 + 0.02) / 6,
              1e-12);
  EXPECT_NEAR(StandardizedUnexpectedEarnings(q, forecast), 0.0, 1e-12);
  EXPECT_GT(StandardizedUnexpectedEarnings(q, forecast + 0.05), 2.0);
}

TEST(GroupTest, SortAndAverage) {
  const std::vector<double> score{0.3, -1.2, 2.5, 0.0, -0.4, 1.1};
  const auto g = SortIntoGroups(score, 3);
  EXPECT_EQ(g, (std::vector<int>{1, 0, 2, 1, 0, 2}));
  const std::vector<double> ret{0.01, -0.02, 0.04, 0.00, -0.01, 0.03};
  const auto means = GroupMeans(ret, g, 3);
  EXPECT_NEAR(means[0], -0.015, 1e-15);
  EXPECT_NEAR(means[1], 0.005, 1e-15);
  EXPECT_NEAR(means[2], 0.035, 1e-15);
}

}  // namespace
}  // namespace investm::ch18
