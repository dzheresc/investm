#include "cpp/core/regression.h"

#include <cmath>
#include <cstddef>
#include <utility>

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

std::optional<OlsDesign> OlsDesign::Create(const Matrix& x, bool intercept) {
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
  // X'X as a sum of outer products of the (contiguous) rows: no transpose
  // copy, and only the upper triangle is accumulated.
  Matrix xtx(k, k);
  for (std::size_t r = 0; r < n; ++r) {
    const std::span<const double> row = design.RowSpan(r);
    for (std::size_t i = 0; i < k; ++i) {
      for (std::size_t j = i; j < k; ++j) xtx(i, j) += row[i] * row[j];
    }
  }
  for (std::size_t i = 0; i < k; ++i) {
    for (std::size_t j = 0; j < i; ++j) xtx(i, j) = xtx(j, i);
  }
  auto xtx_inv = Inverse(xtx);
  if (!xtx_inv.has_value()) return std::nullopt;
  return OlsDesign(std::move(design), *std::move(xtx_inv), intercept);
}

LinearRegression OlsDesign::Fit(std::span<const double> y) const {
  const std::size_t n = design_.rows();
  const std::size_t k = design_.cols();
  INVESTM_CHECK(y.size() == n, "X and y have different lengths");
  Vector xty(k, 0.0);
  for (std::size_t r = 0; r < n; ++r) {
    const std::span<const double> row = design_.RowSpan(r);
    for (std::size_t c = 0; c < k; ++c) xty[c] += row[c] * y[r];
  }
  LinearRegression fit;
  fit.has_intercept = intercept_;
  fit.coefficients = xtx_inv_ * xty;
  fit.residuals.resize(n);
  double sse = 0.0;
  double mean_y = 0.0;
  for (double v : y) mean_y += v;
  mean_y /= static_cast<double>(n);
  double sst = 0.0;
  for (std::size_t r = 0; r < n; ++r) {
    const std::span<const double> row = design_.RowSpan(r);
    double fitted = 0.0;
    for (std::size_t c = 0; c < k; ++c) fitted += row[c] * fit.coefficients[c];
    fit.residuals[r] = y[r] - fitted;
    sse += fit.residuals[r] * fit.residuals[r];
    sst += intercept_ ? (y[r] - mean_y) * (y[r] - mean_y) : y[r] * y[r];
  }
  const double s2 = sse / static_cast<double>(n - k);
  fit.residual_std_dev = std::sqrt(s2);
  fit.r_squared = sst > 0.0 ? 1.0 - sse / sst : 1.0;
  fit.std_errors.resize(k);
  for (std::size_t c = 0; c < k; ++c) {
    fit.std_errors[c] = std::sqrt(s2 * xtx_inv_(c, c));
  }
  return fit;
}

std::optional<LinearRegression> MultipleOls(const Matrix& x,
                                            std::span<const double> y,
                                            bool intercept) {
  INVESTM_CHECK(x.rows() == y.size(), "X and y have different lengths");
  const auto design = OlsDesign::Create(x, intercept);
  if (!design.has_value()) return std::nullopt;
  return design->Fit(y);
}

}  // namespace investm
