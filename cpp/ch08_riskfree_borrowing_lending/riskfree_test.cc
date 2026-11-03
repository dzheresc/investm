#include "cpp/ch08_riskfree_borrowing_lending/riskfree.h"

#include <cmath>
#include <numeric>

#include "gtest/gtest.h"

namespace investm::ch08 {
namespace {

const Vector kMu{0.08, 0.12, 0.15};
Matrix Cov() {
  return CovarianceFromCorrelation(
      Vector{0.15, 0.22, 0.30},
      Matrix({{1.0, 0.3, 0.2}, {0.3, 1.0, 0.4}, {0.2, 0.4, 1.0}}));
}

TEST(RiskfreeTest, CombinationsLieOnAStraightLine) {
  const Combination half = CombineWithRiskfree(0.04, 0.12, 0.20, 0.5);
  EXPECT_NEAR(half.expected_return, 0.08, 1e-15);
  EXPECT_NEAR(half.std_dev, 0.10, 1e-15);
  const Combination levered = CombineWithRiskfree(0.04, 0.12, 0.20, 1.5);
  EXPECT_NEAR(levered.expected_return, 0.16, 1e-15);
  EXPECT_NEAR(SharpeRatio(half.expected_return, half.std_dev, 0.04),
              SharpeRatio(levered.expected_return, levered.std_dev, 0.04),
              1e-14);
}

TEST(RiskfreeTest, TangencyMaximizesSharpeRatio) {
  const double rf = 0.04;
  const auto t = TangencyPortfolio(kMu, Cov(), rf);
  ASSERT_TRUE(t.has_value());
  EXPECT_NEAR(std::accumulate(t->weights.begin(), t->weights.end(), 0.0), 1.0,
              1e-12);
  const double best = SharpeRatio(t->expected_return, t->std_dev, rf);
  const auto frontier = ch07::UnconstrainedFrontier::Create(kMu, Cov());
  for (double e = 0.06; e < 0.25; e += 0.005) {
    const auto p = frontier->ForTargetReturn(e);
    EXPECT_LE(SharpeRatio(p.expected_return, p.std_dev, rf), best + 1e-12);
  }
}

TEST(RiskfreeTest, NoTangencyAboveMinimumVarianceReturn) {
  EXPECT_FALSE(TangencyPortfolio(kMu, Cov(), 0.50).has_value());
}

TEST(RiskfreeTest, NoShortSalesTangencyAgreesWhenWeightsArePositive) {
  const auto free = TangencyPortfolio(kMu, Cov(), 0.04);
  const auto constrained = TangencyPortfolioNoShortSales(kMu, Cov(), 0.04);
  for (std::size_t i = 0; i < 3; ++i) {
    ASSERT_GT(free->weights[i], 0.0);
    EXPECT_NEAR(constrained.weights[i], free->weights[i], 1e-5);
  }
}

TEST(RiskfreeTest, OptimalFractionMaximizesUtility) {
  const auto t = TangencyPortfolio(kMu, Cov(), 0.04);
  const double tau = 0.5;
  const double x = OptimalFractionInTangency(*t, 0.04, tau);
  auto utility = [&](double frac) {
    const Combination c =
        CombineWithRiskfree(0.04, t->expected_return, t->std_dev, frac);
    return c.expected_return - c.std_dev * c.std_dev / tau;
  };
  EXPECT_GT(utility(x), utility(x - 0.01));
  EXPECT_GT(utility(x), utility(x + 0.01));
}

TEST(RiskfreeTest, DifferentRatesRegions) {
  const double rl = 0.03;
  const double rb = 0.07;
  const auto timid = OptimalWithDifferentRates(kMu, Cov(), rl, rb, 0.05);
  ASSERT_TRUE(timid.has_value());
  EXPECT_EQ(timid->region, Region::kLend);
  EXPECT_LT(timid->fraction_risky, 1.0);
  const auto bold = OptimalWithDifferentRates(kMu, Cov(), rl, rb, 10.0);
  EXPECT_EQ(bold->region, Region::kBorrow);
  EXPECT_GT(bold->fraction_risky, 1.0);
  // Somewhere in between the investor holds only risky assets.
  bool found_middle = false;
  for (double tau = 0.05; tau < 10.0; tau *= 1.2) {
    const auto c = OptimalWithDifferentRates(kMu, Cov(), rl, rb, tau);
    found_middle |= c->region == Region::kRiskyOnly;
  }
  EXPECT_TRUE(found_middle);
}

TEST(RiskfreeTest, ExpensiveBorrowingIsNeverUsed) {
  // 50% is above every frontier return, so there is no borrowing tangency.
  const auto bold = OptimalWithDifferentRates(kMu, Cov(), 0.03, 0.50, 10.0);
  ASSERT_TRUE(bold.has_value());
  EXPECT_EQ(bold->region, Region::kRiskyOnly);
}

}  // namespace
}  // namespace investm::ch08
