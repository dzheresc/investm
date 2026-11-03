#include "cpp/ch07_portfolio_analysis/market_model.h"

#include <cmath>
#include <vector>

#include "gtest/gtest.h"

namespace investm::ch07 {
namespace {

TEST(MarketModelTest, FitRecoversParameters) {
  const std::vector<double> index{0.02, -0.01, 0.03, 0.00, -0.02, 0.015};
  std::vector<double> stock;
  const double noise[] = {0.001, -0.002, 0.0015, -0.001, 0.002, -0.0015};
  for (std::size_t t = 0; t < index.size(); ++t) {
    stock.push_back(0.001 + 1.4 * index[t] + noise[t]);
  }
  const MarketModel m = FitMarketModel(stock, index);
  EXPECT_NEAR(m.beta, 1.4, 0.1);
  EXPECT_GT(m.r_squared, 0.95);
  EXPECT_GT(m.residual_variance, 0.0);
}

TEST(MarketModelTest, PortfolioRiskDecomposition) {
  const std::vector<MarketModel> models{{0.0, 0.8, 0.02, 0.0},
                                        {0.0, 1.2, 0.03, 0.0}};
  const std::vector<double> w{0.5, 0.5};
  const PortfolioRisk r = PortfolioRiskFromMarketModels(w, models, 0.04);
  EXPECT_NEAR(r.beta, 1.0, 1e-15);
  EXPECT_NEAR(r.market_variance, 0.04, 1e-15);
  EXPECT_NEAR(r.unique_variance, 0.25 * 0.02 + 0.25 * 0.03, 1e-15);
  // Same total as the implied covariance matrix.
  const Matrix cov = MarketModelCovariance(models, 0.04);
  EXPECT_NEAR(QuadraticForm(w, cov), r.total_variance, 1e-15);
  const Vector mu = MarketModelExpectedReturns(models, 0.10);
  EXPECT_NEAR(mu[1], 0.12, 1e-15);
}

TEST(MarketModelTest, RandomDiversification) {
  EXPECT_NEAR(RandomDiversificationVariance(1, 0.16, 0.04), 0.16, 1e-15);
  EXPECT_NEAR(RandomDiversificationVariance(1000000, 0.16, 0.04), 0.04, 1e-6);
  const Matrix cov(
      {{0.04, 0.01, 0.02}, {0.01, 0.09, 0.03}, {0.02, 0.03, 0.16}});
  const AverageRisk avg = AverageVarianceAndCovariance(cov);
  EXPECT_NEAR(avg.variance, 0.29 / 3.0, 1e-15);
  EXPECT_NEAR(avg.covariance, 0.02, 1e-15);
}

}  // namespace
}  // namespace investm::ch07
