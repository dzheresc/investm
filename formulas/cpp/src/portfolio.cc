#include "investm/formulas/portfolio.h"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <span>

namespace investm::formulas {
namespace {

double WeightedSum(std::span<const double> weights,
                   std::span<const double> values) {
  assert(weights.size() == values.size());
  double total = 0.0;
  for (std::size_t i = 0; i < weights.size(); ++i) {
    total += weights[i] * values[i];
  }
  return total;
}

}  // namespace

double PortfolioExpectedReturn(std::span<const double> weights,
                               std::span<const double> expected_returns) {
  return WeightedSum(weights, expected_returns);
}

double Covariance(double correlation, double std_dev_i, double std_dev_j) {
  return correlation * std_dev_i * std_dev_j;
}

double PortfolioVariance(std::span<const double> weights, const Matrix& cov) {
  assert(cov.size() == weights.size());
  double total = 0.0;
  for (std::size_t i = 0; i < weights.size(); ++i) {
    assert(cov[i].size() == weights.size());
    for (std::size_t j = 0; j < weights.size(); ++j) {
      total += weights[i] * weights[j] * cov[i][j];
    }
  }
  return total;
}

double PortfolioStdDev(std::span<const double> weights, const Matrix& cov) {
  return std::sqrt(PortfolioVariance(weights, cov));
}

double TwoAssetStdDev(double weight_1, double weight_2, double std_dev_1,
                      double std_dev_2, double correlation) {
  return std::sqrt(weight_1 * weight_1 * std_dev_1 * std_dev_1 +
                   weight_2 * weight_2 * std_dev_2 * std_dev_2 +
                   2.0 * weight_1 * weight_2 * correlation * std_dev_1 *
                       std_dev_2);
}

double MarketModelReturn(double alpha, double beta, double index_return,
                         double error) {
  return alpha + beta * index_return + error;
}

double MarketModelBeta(double cov_with_index, double index_variance) {
  return cov_with_index / index_variance;
}

double MarketModelVariance(double beta, double index_variance,
                           double residual_variance) {
  return beta * beta * index_variance + residual_variance;
}

double MarketModelCovariance(double beta_i, double beta_j,
                             double index_variance) {
  return beta_i * beta_j * index_variance;
}

double PortfolioVarianceMarketModel(double portfolio_beta,
                                    double index_variance,
                                    double unique_variance) {
  return portfolio_beta * portfolio_beta * index_variance + unique_variance;
}

double PortfolioBeta(std::span<const double> weights,
                     std::span<const double> betas) {
  return WeightedSum(weights, betas);
}

double PortfolioUniqueVariance(std::span<const double> weights,
                               std::span<const double> residual_variances) {
  assert(weights.size() == residual_variances.size());
  double total = 0.0;
  for (std::size_t i = 0; i < weights.size(); ++i) {
    total += weights[i] * weights[i] * residual_variances[i];
  }
  return total;
}

double CapitalMarketLine(double risk_free, double market_return,
                         double market_std_dev, double portfolio_std_dev) {
  return risk_free +
         (market_return - risk_free) / market_std_dev * portfolio_std_dev;
}

double SecurityMarketLineCov(double risk_free, double market_return,
                             double market_variance, double cov_with_market) {
  return risk_free +
         (market_return - risk_free) / market_variance * cov_with_market;
}

double SecurityMarketLine(double risk_free, double market_return, double beta) {
  return risk_free + (market_return - risk_free) * beta;
}

double CapmBeta(double cov_with_market, double market_variance) {
  return cov_with_market / market_variance;
}

}  // namespace investm::formulas
