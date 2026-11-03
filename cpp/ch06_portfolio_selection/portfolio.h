// Chapter 6, The Portfolio Selection Problem.
// Sections 6.1 (initial and terminal wealth, the rate of return on a
// portfolio) and 6.5 (expected returns and standard deviations of
// portfolios), plus Appendix A (characteristics of probability distributions)
// applied to joint scenarios.

#ifndef CPP_CH06_PORTFOLIO_SELECTION_PORTFOLIO_H_
#define CPP_CH06_PORTFOLIO_SELECTION_PORTFOLIO_H_

#include <span>
#include <vector>

#include "cpp/core/matrix.h"

namespace investm::ch06 {

// 6.1.1 Weights X_i = (shares_i * price_i) / initial wealth.
std::vector<double> WeightsFromHoldings(std::span<const double> shares,
                                        std::span<const double> prices);

// 6.1.1 (W1 - W0) / W0 for a portfolio of holdings.
double PortfolioReturnFromPrices(std::span<const double> shares,
                                 std::span<const double> begin_prices,
                                 std::span<const double> end_values);

// 6.5.1 r_P = sum X_i r_i.
double PortfolioExpectedReturn(std::span<const double> weights,
                               std::span<const double> expected_returns);

// 6.5.2 sigma_P^2 = sum_i sum_j X_i X_j sigma_ij.
double PortfolioVariance(std::span<const double> weights, const Matrix& cov);
double PortfolioStdDev(std::span<const double> weights, const Matrix& cov);

// Two-asset special case:
//   sigma_P^2 = X1^2 s1^2 + X2^2 s2^2 + 2 X1 X2 rho s1 s2.
double TwoAssetStdDev(double x1, double s1, double s2, double rho);

// Appendix A with joint scenarios: rows = states, columns = securities, and
// `probs` the state probabilities. Returns the vector of expected returns
// and the covariance matrix.
struct ScenarioMoments {
  std::vector<double> expected;
  Matrix cov;
};
ScenarioMoments MomentsFromScenarios(const Matrix& returns,
                                     std::span<const double> probs);

// Portfolio return in every scenario.
std::vector<double> ScenarioPortfolioReturns(const Matrix& returns,
                                             std::span<const double> weights);

}  // namespace investm::ch06

#endif  // CPP_CH06_PORTFOLIO_SELECTION_PORTFOLIO_H_
