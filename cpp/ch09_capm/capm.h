// Chapter 9, The Capital Asset Pricing Model.
// Sections 9.2 (the capital market line, separation theorem, market
// portfolio), 9.3 (the security market line), 9.4 (the market model and the
// partitioning of risk), Appendix A (restricted borrowing / zero-beta CAPM,
// liquidity) and Appendix B (derivation of the SML).

#ifndef CPP_CH09_CAPM_CAPM_H_
#define CPP_CH09_CAPM_CAPM_H_

#include <span>

#include "cpp/core/matrix.h"

namespace investm::ch09 {

// 9.2 CML: efficient portfolios satisfy
//   E_p = rf + (E_M - rf) / sigma_M * sigma_p.
double CapitalMarketLine(double rf, double market_return, double market_std_dev,
                         double std_dev);

// 9.3 SML: every security satisfies E_i = rf + (E_M - rf) beta_i.
double SecurityMarketLine(double rf, double market_return, double beta);

// beta_i = sigma_iM / sigma_M^2.
double Beta(double cov_with_market, double market_variance);

// Betas of every asset with respect to the portfolio `market_weights`:
//   beta = S w_M / (w_M' S w_M).
Vector Betas(const Matrix& cov, std::span<const double> market_weights);

// Expected return minus the SML prediction (positive means underpriced).
double Alpha(double expected_return, double rf, double market_return,
             double beta);

// Equilibrium ("implied") expected returns: the returns that make the given
// market weights the tangency portfolio, rf + beta_i (E_M - rf).
Vector ImpliedExpectedReturns(const Matrix& cov,
                              std::span<const double> market_weights, double rf,
                              double market_risk_premium);

// 9.4.2 Partition of a security's variance under the CAPM:
//   sigma_i^2 = beta_i^2 sigma_M^2 (market risk) + sigma_ei^2 (non-market).
struct RiskPartition {
  double market_variance;
  double nonmarket_variance;
};
RiskPartition PartitionRisk(double total_variance, double beta,
                            double market_variance);

// Appendix A.1: with no riskfree borrowing, the SML runs through the expected
// return of the market's zero-beta portfolio Z. For a frontier portfolio p,
// using the frontier constants A, B, C of Ch. 7,
//   E_Z = (B E_p - C) / (A E_p - B).
double ZeroBetaReturn(double a, double b, double c, double frontier_return);
double ZeroBetaSecurityMarketLine(double zero_beta_return, double market_return,
                                  double beta);

// Appendix A.3: a liquidity-adjusted SML. Investors demand `cost_premium`
// per unit of (amortized) trading cost on top of the CAPM return.
double LiquidityAdjustedReturn(double rf, double market_return, double beta,
                               double trading_cost, double cost_premium);

}  // namespace investm::ch09

#endif  // CPP_CH09_CAPM_CAPM_H_
