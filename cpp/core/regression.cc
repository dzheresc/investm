#include "cpp/core/regression.h"

#include <cmath>

#include "cpp/core/check.h"

namespace investm {

double LinearRegression::Predict(std::span<const double> x) const {
  const std::size_t offset = has_intercept ? 1 : 0;
  INVESTM_CHECK(x.size() + offset == coefficients.size(),
                "wrong number of regressors");
  double y = has_intercept ? coefficients[0] : 0.0;
  for (std::size_t j = 0; j < x.size(); ++j) {
    y += coefficients[j + offset] * x[j];
  }
  return y;
}

std::optional<LinearRegression> MultipleOls(const Matrix& x,
                                            std::span<const double> y,
                                            bool intercept) {
  INVESTM_CHECK(x.rows() == y.size(), "X and y have different lengths");
  const std::size_t n = x.rows();
  const std::size_t k = x.cols() + (intercept ? 1 : 0);
  if (n <= k) return std::nullopt;

  // Design matrix with an optional leading column of ones.
  Matrix design(n, k);
  for (std::size_t r = 0; r < n; ++r) {
    std::size_t c = 0;
    if (intercept) design(r, c++) = 1.0;
    for (std::size_t j = 0; j < x.cols(); ++j) design(r, c++) = x(r, j);
  }
  const Matrix xt = design.Transpose();
  const auto xtx_inv = Inverse(xt * design);
  if (!xtx_inv.has_value()) return std::nullopt;

  LinearRegression fit;
  fit.has_intercept = intercept;
  fit.coefficients = *xtx_inv * (xt * y);
  fit.residuals.resize(n);
  double sse = 0.0;
  double mean_y = 0.0;
  for (double v : y) mean_y += v;
  mean_y /= static_cast<double>(n);
  double sst = 0.0;
  for (std::size_t r = 0; r < n; ++r) {
    double fitted = 0.0;
    for (std::size_t c = 0; c < k; ++c) {
      fitted += design(r, c) * fit.coefficients[c];
    }
    fit.residuals[r] = y[r] - fitted;
    sse += fit.residuals[r] * fit.residuals[r];
    sst += intercept ? (y[r] - mean_y) * (y[r] - mean_y) : y[r] * y[r];
  }
  const double s2 = sse / static_cast<double>(n - k);
  fit.residual_std_dev = std::sqrt(s2);
  fit.r_squared = sst > 0.0 ? 1.0 - sse / sst : 1.0;
  fit.std_errors.resize(k);
  for (std::size_t c = 0; c < k; ++c) {
    fit.std_errors[c] = std::sqrt(s2 * (*xtx_inv)(c, c));
  }
  return fit;
}

}  // namespace investm
