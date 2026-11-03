// Chapter 16, Common Stocks.
// Section 16.7: betas. Estimating beta from returns, adjusting historical
// betas toward the mean, and the effect of financial leverage on beta.

#ifndef CPP_CH16_COMMON_STOCKS_BETAS_H_
#define CPP_CH16_COMMON_STOCKS_BETAS_H_

#include <span>
#include <vector>

#include "cpp/core/stats.h"

namespace investm::ch16 {

// Ex-post beta: OLS slope of security on market returns (with standard
// error, alpha and R^2).
stats::SimpleRegression EstimateBeta(std::span<const double> security,
                                     std::span<const double> market);

// Betas over consecutive non-overlapping windows of `window` observations.
std::vector<double> RollingBetas(std::span<const double> security,
                                 std::span<const double> market,
                                 std::size_t window);

// Mechanical adjustment toward a prior: weight * raw + (1 - weight) * prior.
// With weight 2/3 and prior 1 this is the common "adjusted beta".
double ShrinkBeta(double raw, double weight = 2.0 / 3.0, double prior = 1.0);

// Bayesian (Vasicek) adjustment: precision-weighted average of the raw
// estimate and a cross-sectional prior.
double VasicekBeta(double raw, double raw_std_error, double prior_mean,
                   double prior_std_dev);

// Cross-sectional regression-to-the-mean fit: regress betas from a later
// period on betas from an earlier one; slope < 1 shows the tendency.
stats::SimpleRegression BetaPersistence(std::span<const double> earlier,
                                        std::span<const double> later);

// Financial leverage (Hamada with corporate tax, riskless debt):
//   beta_levered = beta_unlevered (1 + (1 - t) D / E).
double LeveredBeta(double unlevered_beta, double debt_to_equity,
                   double tax_rate);
double UnleveredBeta(double levered_beta, double debt_to_equity,
                     double tax_rate);

}  // namespace investm::ch16

#endif  // CPP_CH16_COMMON_STOCKS_BETAS_H_
