#include "cpp/ch06_portfolio_selection/portfolio.h"

#include <cmath>
#include <vector>

#include "cpp/core/check.h"

namespace investm::ch06 {

std::vector<double> WeightsFromHoldings(std::span<const double> shares,
                                        std::span<const double> prices) {
  INVESTM_CHECK(shares.size() == prices.size(), "size mismatch");
  double total = 0.0;
  for (std::size_t i = 0; i < shares.size(); ++i)
    total += shares[i] * prices[i];
  INVESTM_CHECK(total != 0.0, "portfolio has zero value");
  std::vector<double> w(shares.size());
  for (std::size_t i = 0; i < shares.size(); ++i) {
    w[i] = shares[i] * prices[i] / total;
  }
  return w;
}

double PortfolioReturnFromPrices(std::span<const double> shares,
                                 std::span<const double> begin_prices,
                                 std::span<const double> end_values) {
  INVESTM_CHECK(shares.size() == begin_prices.size() &&
                    shares.size() == end_values.size(),
                "size mismatch");
  double w0 = 0.0;
  double w1 = 0.0;
  for (std::size_t i = 0; i < shares.size(); ++i) {
    w0 += shares[i] * begin_prices[i];
    w1 += shares[i] * end_values[i];
  }
  INVESTM_CHECK(w0 > 0.0, "initial wealth must be positive");
  return (w1 - w0) / w0;
}

double PortfolioExpectedReturn(std::span<const double> weights,
                               std::span<const double> expected_returns) {
  return Dot(weights, expected_returns);
}

double PortfolioVariance(std::span<const double> weights, const Matrix& cov) {
  return QuadraticForm(weights, cov);
}

double PortfolioStdDev(std::span<const double> weights, const Matrix& cov) {
  return std::sqrt(PortfolioVariance(weights, cov));
}

double TwoAssetStdDev(double x1, double s1, double s2, double rho) {
  const double x2 = 1.0 - x1;
  return std::sqrt(x1 * x1 * s1 * s1 + x2 * x2 * s2 * s2 +
                   2.0 * x1 * x2 * rho * s1 * s2);
}

ScenarioMoments MomentsFromScenarios(const Matrix& returns,
                                     std::span<const double> probs) {
  INVESTM_CHECK(returns.rows() == probs.size(), "one probability per state");
  const std::size_t n = returns.cols();
  ScenarioMoments m{std::vector<double>(n, 0.0), Matrix(n, n)};
  for (std::size_t s = 0; s < returns.rows(); ++s) {
    for (std::size_t i = 0; i < n; ++i)
      m.expected[i] += probs[s] * returns(s, i);
  }
  for (std::size_t s = 0; s < returns.rows(); ++s) {
    for (std::size_t i = 0; i < n; ++i) {
      const double di = returns(s, i) - m.expected[i];
      for (std::size_t j = 0; j < n; ++j) {
        m.cov(i, j) += probs[s] * di * (returns(s, j) - m.expected[j]);
      }
    }
  }
  return m;
}

std::vector<double> ScenarioPortfolioReturns(const Matrix& returns,
                                             std::span<const double> weights) {
  return returns * weights;
}

}  // namespace investm::ch06
