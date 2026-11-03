// Chapter 7, Portfolio Analysis.
// Sections 7.3 (the market model: random error terms, beta, actual returns),
// 7.4 (diversification: total, market and unique risk; random versus
// efficient diversification) and Appendix B (determining the inputs needed
// for the efficient set from the market model).

#ifndef CPP_CH07_PORTFOLIO_ANALYSIS_MARKET_MODEL_H_
#define CPP_CH07_PORTFOLIO_ANALYSIS_MARKET_MODEL_H_

#include <span>
#include <vector>

#include "cpp/core/matrix.h"

namespace investm::ch07 {

// r_i = alpha_i + beta_i r_I + e_i, estimated by OLS on historical returns.
struct MarketModel {
  double alpha = 0.0;
  double beta = 0.0;
  double residual_variance = 0.0;  // sigma^2_{e_i}
  double r_squared = 0.0;

  // 7.4: total variance = beta^2 sigma_I^2 + sigma_e^2.
  double MarketVariance(double index_variance) const {
    return beta * beta * index_variance;
  }
  double TotalVariance(double index_variance) const {
    return MarketVariance(index_variance) + residual_variance;
  }
};

MarketModel FitMarketModel(std::span<const double> security,
                           std::span<const double> index);

// 7.4.1-7.4.3 Risk of a portfolio of securities described by market models,
// assuming the residuals are uncorrelated across securities.
struct PortfolioRisk {
  double beta = 0.0;
  double alpha = 0.0;
  double market_variance = 0.0;  // beta_P^2 sigma_I^2
  double unique_variance = 0.0;  // sum X_i^2 sigma^2_{e_i}
  double total_variance = 0.0;
};
PortfolioRisk PortfolioRiskFromMarketModels(std::span<const double> weights,
                                            std::span<const MarketModel> models,
                                            double index_variance);

// Appendix B: the full covariance matrix implied by the market model,
//   sigma_ij = beta_i beta_j sigma_I^2 (+ sigma^2_{e_i} on the diagonal),
// and the expected returns alpha_i + beta_i E[r_I]. Needs 3N + 2 estimates
// instead of N(N + 3)/2.
Matrix MarketModelCovariance(std::span<const MarketModel> models,
                             double index_variance);
Vector MarketModelExpectedReturns(std::span<const MarketModel> models,
                                  double index_expected_return);

// 7.4.5 Random diversification: expected variance of an equally weighted
// portfolio of N randomly chosen securities with the given average variance
// and average covariance:  avg_var / N + (1 - 1/N) avg_cov.
double RandomDiversificationVariance(int n, double average_variance,
                                     double average_covariance);

// Average variance and average pairwise covariance of a covariance matrix.
struct AverageRisk {
  double variance;
  double covariance;
};
AverageRisk AverageVarianceAndCovariance(const Matrix& cov);

}  // namespace investm::ch07

#endif  // CPP_CH07_PORTFOLIO_ANALYSIS_MARKET_MODEL_H_
