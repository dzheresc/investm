#include "cpp/ch07_portfolio_analysis/market_model.h"

#include "cpp/core/check.h"
#include "cpp/core/stats.h"

namespace investm::ch07 {

MarketModel FitMarketModel(std::span<const double> security,
                           std::span<const double> index) {
  const stats::SimpleRegression fit = stats::Ols(index, security);
  return {fit.alpha, fit.beta, fit.residual_std_dev * fit.residual_std_dev,
          fit.r_squared};
}

PortfolioRisk PortfolioRiskFromMarketModels(std::span<const double> weights,
                                            std::span<const MarketModel> models,
                                            double index_variance) {
  INVESTM_CHECK(weights.size() == models.size(), "size mismatch");
  PortfolioRisk risk;
  for (std::size_t i = 0; i < weights.size(); ++i) {
    risk.beta += weights[i] * models[i].beta;
    risk.alpha += weights[i] * models[i].alpha;
    risk.unique_variance +=
        weights[i] * weights[i] * models[i].residual_variance;
  }
  risk.market_variance = risk.beta * risk.beta * index_variance;
  risk.total_variance = risk.market_variance + risk.unique_variance;
  return risk;
}

Matrix MarketModelCovariance(std::span<const MarketModel> models,
                             double index_variance) {
  const std::size_t n = models.size();
  Matrix cov(n, n);
  for (std::size_t i = 0; i < n; ++i) {
    for (std::size_t j = 0; j < n; ++j) {
      cov(i, j) = models[i].beta * models[j].beta * index_variance;
    }
    cov(i, i) += models[i].residual_variance;
  }
  return cov;
}

Vector MarketModelExpectedReturns(std::span<const MarketModel> models,
                                  double index_expected_return) {
  Vector mu;
  mu.reserve(models.size());
  for (const MarketModel& m : models) {
    mu.push_back(m.alpha + m.beta * index_expected_return);
  }
  return mu;
}

double RandomDiversificationVariance(int n, double average_variance,
                                     double average_covariance) {
  INVESTM_CHECK(n >= 1, "need at least one security");
  const double nn = n;
  return average_variance / nn + (1.0 - 1.0 / nn) * average_covariance;
}

AverageRisk AverageVarianceAndCovariance(const Matrix& cov) {
  const std::size_t n = cov.rows();
  INVESTM_CHECK(n >= 2 && cov.cols() == n, "need a square matrix, n >= 2");
  double var = 0.0;
  double covariance = 0.0;
  for (std::size_t i = 0; i < n; ++i) {
    var += cov(i, i);
    for (std::size_t j = i + 1; j < n; ++j) covariance += cov(i, j);
  }
  const double nn = static_cast<double>(n);
  return {var / nn, covariance / (nn * (nn - 1.0) / 2.0)};
}

}  // namespace investm::ch07
