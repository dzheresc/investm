#include "cpp/ch09_capm/capm.h"

#include <cmath>
#include <vector>

#include "cpp/ch07_portfolio_analysis/efficient_set.h"
#include "cpp/ch08_riskfree_borrowing_lending/riskfree.h"
#include "gtest/gtest.h"

namespace investm::ch09 {
namespace {

const Vector kMu{0.08, 0.12, 0.15};
Matrix Cov() {
  return CovarianceFromCorrelation(
      Vector{0.15, 0.22, 0.30},
      Matrix({{1.0, 0.3, 0.2}, {0.3, 1.0, 0.4}, {0.2, 0.4, 1.0}}));
}

TEST(CapmTest, EverySecurityLiesOnTheSmlWhenMarketIsTangency) {
  const double rf = 0.04;
  const auto m = ch08::TangencyPortfolio(kMu, Cov(), rf).value();
  const Vector betas = Betas(Cov(), m.weights);
  for (std::size_t i = 0; i < kMu.size(); ++i) {
    EXPECT_NEAR(kMu[i], SecurityMarketLine(rf, m.expected_return, betas[i]),
                1e-12);
    EXPECT_NEAR(Alpha(kMu[i], rf, m.expected_return, betas[i]), 0.0, 1e-12);
  }
  // The market's own beta is one, and it lies on the CML.
  double market_beta = 0.0;
  for (std::size_t i = 0; i < 3; ++i) market_beta += m.weights[i] * betas[i];
  EXPECT_NEAR(market_beta, 1.0, 1e-12);
  EXPECT_NEAR(CapitalMarketLine(rf, m.expected_return, m.std_dev, m.std_dev),
              m.expected_return, 1e-15);
}

TEST(CapmTest, ImpliedReturnsMakeMarketTheTangency) {
  const Vector w{0.5, 0.3, 0.2};
  const Vector mu = ImpliedExpectedReturns(Cov(), w, 0.03, 0.05);
  const auto t = ch08::TangencyPortfolio(mu, Cov(), 0.03).value();
  for (std::size_t i = 0; i < 3; ++i) EXPECT_NEAR(t.weights[i], w[i], 1e-10);
}

TEST(CapmTest, RiskPartition) {
  const RiskPartition p = PartitionRisk(0.09, 1.2, 0.04);
  EXPECT_NEAR(p.market_variance, 0.0576, 1e-15);
  EXPECT_NEAR(p.nonmarket_variance, 0.0324, 1e-15);
  EXPECT_NEAR(Beta(0.03, 0.04), 0.75, 1e-15);
}

TEST(CapmTest, ZeroBetaPortfolioIsUncorrelatedWithMarket) {
  const auto f = ch07::UnconstrainedFrontier::Create(kMu, Cov()).value();
  const double e_m = 0.13;
  const double e_z = ZeroBetaReturn(f.a(), f.b(), f.c(), e_m);
  const auto m = f.ForTargetReturn(e_m);
  const auto z = f.ForTargetReturn(e_z);
  EXPECT_NEAR(QuadraticForm(m.weights, Cov(), z.weights), 0.0, 1e-12);
  // Every asset lies on the zero-beta SML.
  const Vector betas = Betas(Cov(), m.weights);
  for (std::size_t i = 0; i < 3; ++i) {
    EXPECT_NEAR(kMu[i], ZeroBetaSecurityMarketLine(e_z, e_m, betas[i]), 1e-12);
  }
}

TEST(CapmTest, LiquidityPremium) {
  EXPECT_NEAR(LiquidityAdjustedReturn(0.03, 0.09, 1.0, 0.02, 0.5), 0.10, 1e-15);
}

}  // namespace
}  // namespace investm::ch09
