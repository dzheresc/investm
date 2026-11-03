// Multiple linear regression by ordinary least squares.
//
// Used for multi-factor models (Ch. 10), APT estimation (Ch. 11) and
// performance attribution (Ch. 24).

#ifndef CPP_CORE_REGRESSION_H_
#define CPP_CORE_REGRESSION_H_

#include <cstddef>
#include <optional>
#include <span>
#include <utility>

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

// Least squares against a fixed set of regressors. Create() builds the
// design matrix and (X'X)^-1 once; each Fit() is then O(T K) for T
// observations and K coefficients. Use it when many series are regressed
// on the same factors (time-series factor models, Ch. 10).
class OlsDesign {
 public:
  // Returns nullopt if X'X is singular or there are not more observations
  // than coefficients.
  static std::optional<OlsDesign> Create(const Matrix& x,
                                         bool intercept = true);

  // y must have one entry per row of the x passed to Create().
  LinearRegression Fit(std::span<const double> y) const;

  std::size_t num_observations() const { return design_.rows(); }

 private:
  OlsDesign(Matrix design, Matrix xtx_inv, bool intercept)
      : design_(std::move(design)),
        xtx_inv_(std::move(xtx_inv)),
        intercept_(intercept) {}

  Matrix design_;   // T x K, with a leading column of ones if intercept_.
  Matrix xtx_inv_;  // K x K.
  bool intercept_;
};

// Regresses y on the columns of x. Returns nullopt if X'X is singular or there
// are not more observations than coefficients.
std::optional<LinearRegression> MultipleOls(const Matrix& x,
                                            std::span<const double> y,
                                            bool intercept = true);

}  // namespace investm

#endif  // CPP_CORE_REGRESSION_H_
