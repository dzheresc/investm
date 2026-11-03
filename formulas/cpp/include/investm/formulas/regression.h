// Simple linear regression of Y on X: equations 38 to 43. Functions that
// take both series require equal lengths of at least two (three for the
// residual standard deviation) and return NaN otherwise.

#ifndef INVESTM_FORMULAS_REGRESSION_H_
#define INVESTM_FORMULAS_REGRESSION_H_

#include <span>

namespace investm::formulas {

// Eq. 38: (T sum XY - sum Y sum X) / (T sum X^2 - (sum X)^2).
double RegressionBeta(std::span<const double> x, std::span<const double> y);

// Eq. 39: sum Y / T - beta sum X / T.
double RegressionAlpha(std::span<const double> x, std::span<const double> y,
                       double beta);

// Eq. 40: [(sum Y^2 - alpha sum Y - beta sum XY) / (T - 2)]^(1/2).
double RegressionResidualStdDev(std::span<const double> x,
                                std::span<const double> y, double alpha,
                                double beta);

// Eq. 41: sigma_e / [sum X^2 - (sum X)^2 / T]^(1/2).
double RegressionBetaStdError(std::span<const double> x,
                              double residual_std_dev);

// Eq. 42: sigma_e / [T - (sum X)^2 / sum X^2]^(1/2).
double RegressionAlphaStdError(std::span<const double> x,
                               double residual_std_dev);

// Eq. 43: sample correlation coefficient in the sums form.
double CorrelationCoefficient(std::span<const double> x,
                              std::span<const double> y);

}  // namespace investm::formulas

#endif  // INVESTM_FORMULAS_REGRESSION_H_
