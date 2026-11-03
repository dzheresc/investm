#include "cpp/core/stats.h"

#include <cmath>
#include <vector>

#include "gtest/gtest.h"

namespace investm::stats {
namespace {

TEST(StatsTest, DescriptiveStatistics) {
  const std::vector<double> x{1, 2, 3, 4, 5};
  const std::vector<double> y{2, 4, 6, 8, 10};
  EXPECT_DOUBLE_EQ(Mean(x), 3.0);
  EXPECT_DOUBLE_EQ(Variance(x), 2.5);
  EXPECT_DOUBLE_EQ(Variance(x, 0), 2.0);
  EXPECT_DOUBLE_EQ(Covariance(x, y), 5.0);
  EXPECT_NEAR(Correlation(x, y), 1.0, 1e-15);
}

TEST(StatsTest, ProbabilityWeightedMoments) {
  const std::vector<double> r{-0.10, 0.05, 0.20};
  const std::vector<double> p{0.25, 0.50, 0.25};
  EXPECT_NEAR(Expected(r, p), 0.05, 1e-15);
  EXPECT_NEAR(ExpectedCovariance(r, r, p), 0.01125, 1e-15);
}

TEST(StatsTest, OlsRecoversExactLine) {
  const std::vector<double> x{0.01, -0.02, 0.03, 0.00, 0.015};
  std::vector<double> y;
  for (double xi : x) y.push_back(0.002 + 1.3 * xi);
  const SimpleRegression r = Ols(x, y);
  EXPECT_NEAR(r.alpha, 0.002, 1e-14);
  EXPECT_NEAR(r.beta, 1.3, 1e-12);
  EXPECT_NEAR(r.r_squared, 1.0, 1e-12);
}

TEST(StatsTest, NormalDistribution) {
  EXPECT_DOUBLE_EQ(NormCdf(0.0), 0.5);
  EXPECT_NEAR(NormCdf(1.96), 0.9750021048517795, 1e-12);
  EXPECT_NEAR(NormPdf(0.0), 0.3989422804014327, 1e-15);
}

TEST(StatsTest, GeometricMeanAndAutocorrelation) {
  const std::vector<double> r{0.10, -0.10};
  EXPECT_NEAR(GeometricMeanReturn(r), std::sqrt(0.99) - 1.0, 1e-15);
  const std::vector<double> alt{1, -1, 1, -1, 1, -1, 1, -1};
  EXPECT_LT(Autocorrelation(alt, 1), -0.8);
}

TEST(StatsDeathTest, MismatchedSizesAbort) {
  const std::vector<double> x{1, 2, 3};
  const std::vector<double> y{1, 2};
  EXPECT_DEATH(Covariance(x, y), "size mismatch");
}

}  // namespace
}  // namespace investm::stats
