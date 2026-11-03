// Multiple linear regression by ordinary least squares.
//
// Used for multi-factor models (Ch. 10), APT estimation (Ch. 11) and
// performance attribution (Ch. 24).

#ifndef CPP_CORE_REGRESSION_H_
#define CPP_CORE_REGRESSION_H_

#include <optional>
#include <span>

#include "cpp/core/matrix.h"

namespace investm {

struct LinearRegression {
  // Intercept first (when fitted), then one slope per column of X.
  Vector coefficients;
  Vector std_errors;
  Vector residuals;
  double r_squared = 0.0;
  double residual_std_dev = 0.0;

  // Fitted value for one observation of the regressors (no intercept term in
  // `x`; it is added when the model has one).
  double Predict(std::span<const double> x) const;
  bool has_intercept = true;
};

// Regresses y on the columns of x. Returns nullopt if X'X is singular or there
// are not more observations than coefficients.
std::optional<LinearRegression> MultipleOls(const Matrix& x,
                                            std::span<const double> y,
                                            bool intercept = true);

}  // namespace investm

#endif  // CPP_CORE_REGRESSION_H_
