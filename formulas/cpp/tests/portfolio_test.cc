#include "investm/formulas/portfolio.h"

#include <gtest/gtest.h>

#include <vector>

namespace investm::formulas {
namespace {

const std::vector<double> kWeights = {0.5, 0.3, 0.2};
const std::vector<double> kStdDevs = {0.20, 0.15, 0.10};

TEST(PortfolioTest, ExpectedReturn) {
  const std::vector<double> r = {0.1, 0.08, 0.05};
  EXPECT_NEAR(PortfolioExpectedReturn(kWeights, r), 0.084, 1e-12);
}

TEST(PortfolioTest, TwoAssetMatchesGeneralFormula) {
  const std::vector<double> w = {0.6, 0.4};
  const Matrix cov = {{Covariance(1.0, 0.2, 0.2), Covariance(0.25, 0.2, 0.3)},
                      {Covariance(0.25, 0.3, 0.2), Covariance(1.0, 0.3, 0.3)}};
  EXPECT_NEAR(TwoAssetStdDev(0.6, 0.4, 0.2, 0.3, 0.25), PortfolioStdDev(w, cov),
              1e-12);
}

TEST(PortfolioTest, PerfectCorrelationGivesWeightedStdDev) {
  Matrix cov(3, std::vector<double>(3));
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j)
      cov[i][j] = Covariance(1.0, kStdDevs[i], kStdDevs[j]);
  }
  EXPECT_NEAR(PortfolioStdDev(kWeights, cov), 0.165, 1e-12);
}

TEST(PortfolioTest, MarketModelMatchesFullCovariance) {
  const std::vector<double> betas = {1.2, 0.8, 1.0};
  const std::vector<double> resid = {0.01, 0.02, 0.015};
  const double var_index = 0.04;
  EXPECT_NEAR(MarketModelReturn(0.01, 1.2, 0.1), 0.13, 1e-12);
  EXPECT_NEAR(MarketModelBeta(0.048, var_index), 1.2, 1e-12);
  EXPECT_NEAR(MarketModelVariance(1.2, var_index, 0.01), 0.0676, 1e-12);

  Matrix cov(3, std::vector<double>(3));
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      cov[i][j] = MarketModelCovariance(betas[i], betas[j], var_index) +
                  (i == j ? resid[i] : 0.0);
    }
  }
  const double beta_p = PortfolioBeta(kWeights, betas);
  EXPECT_NEAR(beta_p, 1.04, 1e-12);
  const double unique = PortfolioUniqueVariance(kWeights, resid);
  EXPECT_NEAR(PortfolioVarianceMarketModel(beta_p, var_index, unique),
              PortfolioVariance(kWeights, cov), 1e-12);
}

TEST(PortfolioTest, CapitalAndSecurityMarketLines) {
  EXPECT_NEAR(CapitalMarketLine(0.04, 0.10, 0.2, 0.1), 0.07, 1e-12);
  const double beta = CapmBeta(0.048, 0.04);
  EXPECT_NEAR(beta, 1.2, 1e-12);
  EXPECT_NEAR(SecurityMarketLine(0.04, 0.10, beta), 0.112, 1e-12);
  EXPECT_NEAR(SecurityMarketLineCov(0.04, 0.10, 0.04, 0.048), 0.112, 1e-12);
}

}  // namespace
}  // namespace investm::formulas
