#include "cpp/ch07_portfolio_analysis/efficient_set.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

#include "gtest/gtest.h"

namespace investm::ch07 {
namespace {

const Vector kMu{0.08, 0.12, 0.15};
Matrix Cov() {
  return CovarianceFromCorrelation(
      Vector{0.15, 0.22, 0.30},
      Matrix({{1.0, 0.3, 0.2}, {0.3, 1.0, 0.4}, {0.2, 0.4, 1.0}}));
}

double Sum(const Vector& v) { return std::accumulate(v.begin(), v.end(), 0.0); }

TEST(UnconstrainedFrontierTest, MinimumVarianceAndTargets) {
  const auto f = UnconstrainedFrontier::Create(kMu, Cov());
  ASSERT_TRUE(f.has_value());
  const PortfolioPoint mv = f->MinimumVariance();
  EXPECT_NEAR(Sum(mv.weights), 1.0, 1e-12);
  EXPECT_NEAR(mv.std_dev * mv.std_dev, 1.0 / f->a(), 1e-12);
  EXPECT_NEAR(mv.expected_return, f->b() / f->a(), 1e-12);
  for (double target : {0.09, 0.12, 0.20}) {
    const PortfolioPoint p = f->ForTargetReturn(target);
    EXPECT_NEAR(Sum(p.weights), 1.0, 1e-12);
    EXPECT_NEAR(p.expected_return, target, 1e-12);
    EXPECT_NEAR(p.std_dev * p.std_dev, f->VarianceAt(target), 1e-12);
    EXPECT_GE(p.std_dev, mv.std_dev - 1e-12);
  }
}

TEST(UnconstrainedFrontierTest, OptimalPortfolioLiesOnFrontier) {
  const auto f = UnconstrainedFrontier::Create(kMu, Cov());
  ASSERT_TRUE(f.has_value());
  const PortfolioPoint opt = f->Optimal(0.5);
  const PortfolioPoint same = f->ForTargetReturn(opt.expected_return);
  for (std::size_t i = 0; i < 3; ++i) {
    EXPECT_NEAR(opt.weights[i], same.weights[i], 1e-12);
  }
  // tau = 0 is the minimum-variance portfolio.
  EXPECT_NEAR(f->Optimal(0.0).std_dev, f->MinimumVariance().std_dev, 1e-14);
}

TEST(UnconstrainedFrontierTest, DegenerateInputsReturnNullopt) {
  EXPECT_FALSE(UnconstrainedFrontier::Create(Vector{0.1, 0.1, 0.1}, Cov()));
  EXPECT_FALSE(UnconstrainedFrontier::Create(Vector{0.1, 0.2},
                                             Matrix({{1.0, 1.0}, {1.0, 1.0}})));
}

TEST(SimplexTest, ProjectionProperties) {
  const Vector w = ProjectOntoSimplex(Vector{0.5, 0.9, -1.0});
  EXPECT_NEAR(Sum(w), 1.0, 1e-15);
  EXPECT_DOUBLE_EQ(w[2], 0.0);
  EXPECT_NEAR(w[0], 0.3, 1e-15);
  EXPECT_NEAR(w[1], 0.7, 1e-15);
  const Vector inside{0.2, 0.3, 0.5};
  const Vector same = ProjectOntoSimplex(inside);
  for (std::size_t i = 0; i < 3; ++i) EXPECT_NEAR(same[i], inside[i], 1e-15);
}

TEST(NoShortSalesTest, MatchesUnconstrainedWhenWeightsArePositive) {
  const auto f = UnconstrainedFrontier::Create(kMu, Cov());
  const PortfolioPoint free = f->Optimal(0.3);
  ASSERT_TRUE(std::all_of(free.weights.begin(), free.weights.end(),
                          [](double w) { return w > 0.0; }));
  const PortfolioPoint qp = OptimalNoShortSales(kMu, Cov(), 0.3);
  for (std::size_t i = 0; i < 3; ++i) {
    EXPECT_NEAR(qp.weights[i], free.weights[i], 1e-8);
  }
}

TEST(NoShortSalesTest, SatisfiesKktConditions) {
  // High risk tolerance pushes the unconstrained solution to short sales.
  const double tau = 3.0;
  const Matrix cov = Cov();
  const PortfolioPoint p = OptimalNoShortSales(kMu, cov, tau);
  EXPECT_NEAR(Sum(p.weights), 1.0, 1e-12);
  const Vector sw = cov * p.weights;
  double nu = 1e300;
  for (std::size_t i = 0; i < 3; ++i) {
    EXPECT_GE(p.weights[i], 0.0);
    if (p.weights[i] > 1e-9) nu = std::min(nu, 2 * sw[i] - tau * kMu[i]);
  }
  for (std::size_t i = 0; i < 3; ++i) {
    const double g = 2 * sw[i] - tau * kMu[i];
    if (p.weights[i] > 1e-9) {
      EXPECT_NEAR(g, nu, 1e-7);
    } else {
      EXPECT_GE(g, nu - 1e-7);
    }
  }
}

TEST(NoShortSalesTest, VeryHighToleranceHoldsTopReturnOnly) {
  const PortfolioPoint p = OptimalNoShortSales(kMu, Cov(), 100.0);
  EXPECT_NEAR(p.weights[2], 1.0, 1e-9);
}

TEST(TwoAssetTest, MinimumVarianceWeight) {
  // Uncorrelated: weights inversely proportional to variance.
  EXPECT_NEAR(TwoAssetMinVarianceWeight(0.2, 0.4, 0.0), 0.16 / 0.20, 1e-15);
  // Perfect negative correlation: zero risk at x1 = s2 / (s1 + s2).
  EXPECT_NEAR(TwoAssetMinVarianceWeight(0.2, 0.4, -1.0), 0.4 / 0.6, 1e-15);
}

}  // namespace
}  // namespace investm::ch07
