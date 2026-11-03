// Portfolio theory, the market model and the CAPM: equations 6 to 20.

#ifndef INVESTM_FORMULAS_PORTFOLIO_H_
#define INVESTM_FORMULAS_PORTFOLIO_H_

#include <span>
#include <vector>

namespace investm::formulas {

// A dense N x N matrix stored as rows.
using Matrix = std::vector<std::vector<double>>;

// Eq. 6: sum of X_i * r_i. The spans must have equal length.
double PortfolioExpectedReturn(std::span<const double> weights,
                               std::span<const double> expected_returns);

// Eq. 7: sigma_ij = rho_ij * sigma_i * sigma_j.
double Covariance(double correlation, double std_dev_i, double std_dev_j);

// Eq. 8 squared, and eq. 18 with market weights:
// sum_i sum_j X_i X_j sigma_ij. `cov` must be N x N.
double PortfolioVariance(std::span<const double> weights, const Matrix& cov);

// Eq. 8: square root of PortfolioVariance.
double PortfolioStdDev(std::span<const double> weights, const Matrix& cov);

// Eq. 9: standard deviation of a two-asset portfolio.
double TwoAssetStdDev(double weight_1, double weight_2, double std_dev_1,
                      double std_dev_2, double correlation);

// Eq. 10: r_i = alpha_i + beta_i * r_I + epsilon_i.
double MarketModelReturn(double alpha, double beta, double index_return,
                         double error = 0.0);

// Eq. 11: beta_i = sigma_iI / sigma_I^2.
double MarketModelBeta(double cov_with_index, double index_variance);

// Eq. 12: sigma_i^2 = beta_i^2 sigma_I^2 + sigma_ei^2.
double MarketModelVariance(double beta, double index_variance,
                           double residual_variance);

// Eq. 13: sigma_ij = beta_i beta_j sigma_I^2.
double MarketModelCovariance(double beta_i, double beta_j,
                             double index_variance);

// Eq. 14: sigma_p^2 = beta_p^2 sigma_I^2 + sigma_ep^2.
double PortfolioVarianceMarketModel(double portfolio_beta,
                                    double index_variance,
                                    double unique_variance);

// Eq. 15: beta_p = sum of X_i beta_i.
double PortfolioBeta(std::span<const double> weights,
                     std::span<const double> betas);

// Eq. 16: sigma_ep^2 = sum of X_i^2 sigma_ei^2.
double PortfolioUniqueVariance(std::span<const double> weights,
                               std::span<const double> residual_variances);

// Eq. 17: r_p = r_f + [(r_M - r_f) / sigma_M] sigma_p.
double CapitalMarketLine(double risk_free, double market_return,
                         double market_std_dev, double portfolio_std_dev);

// Eq. 19, covariance version: r_f + [(r_M - r_f) / sigma_M^2] sigma_iM.
double SecurityMarketLineCov(double risk_free, double market_return,
                             double market_variance, double cov_with_market);

// Eq. 19, beta version: r_i = r_f + (r_M - r_f) beta_i.
double SecurityMarketLine(double risk_free, double market_return, double beta);

// Eq. 20: beta_iM = sigma_iM / sigma_M^2.
double CapmBeta(double cov_with_market, double market_variance);

}  // namespace investm::formulas

#endif  // INVESTM_FORMULAS_PORTFOLIO_H_
