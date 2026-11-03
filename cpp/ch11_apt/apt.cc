#include "cpp/ch11_apt/apt.h"

#include <cmath>

#include "cpp/core/check.h"
#include "cpp/core/regression.h"

namespace investm::ch11 {
namespace {

// A = [1 | B], N x (K + 1).
Matrix WithOnes(const Matrix& b) {
  Matrix a(b.rows(), b.cols() + 1);
  for (std::size_t i = 0; i < b.rows(); ++i) {
    a(i, 0) = 1.0;
    for (std::size_t k = 0; k < b.cols(); ++k) a(i, k + 1) = b(i, k);
  }
  return a;
}

}  // namespace

bool IsArbitragePortfolio(std::span<const double> x, const Matrix& b,
                          std::span<const double> mu, double tolerance) {
  INVESTM_CHECK(x.size() == b.rows() && x.size() == mu.size(),
                "dimension mismatch");
  double total = 0.0;
  for (double v : x) total += v;
  if (std::fabs(total) > tolerance) return false;
  for (double s : b.Transpose() * x) {
    if (std::fabs(s) > tolerance) return false;
  }
  return Dot(x, mu) > tolerance;
}

std::optional<Vector> FindArbitragePortfolio(std::span<const double> mu,
                                             const Matrix& b, double scale,
                                             double tolerance) {
  INVESTM_CHECK(mu.size() == b.rows(), "dimension mismatch");
  const Matrix a = WithOnes(b);
  const Matrix at = a.Transpose();
  const auto coef = Solve(at * a, at * mu);
  if (!coef.has_value()) return std::nullopt;
  // Residual of mu after projecting onto the columns of A.
  Vector x(mu.begin(), mu.end());
  const Vector fitted = a * *coef;
  double purchases = 0.0;
  for (std::size_t i = 0; i < x.size(); ++i) {
    x[i] -= fitted[i];
    if (x[i] > 0.0) purchases += x[i];
  }
  if (purchases <= tolerance) return std::nullopt;
  for (double& v : x) v *= scale / purchases;
  return x;
}

std::optional<AptPricing> FitAptPricing(std::span<const double> mu,
                                        const Matrix& b) {
  const auto fit = MultipleOls(b, mu);
  if (!fit.has_value()) return std::nullopt;
  AptPricing p;
  p.lambda0 = fit->coefficients[0];
  p.lambdas.assign(fit->coefficients.begin() + 1, fit->coefficients.end());
  p.pricing_errors = fit->residuals;
  return p;
}

double AptExpectedReturn(double lambda0, std::span<const double> lambdas,
                         std::span<const double> sensitivities) {
  return lambda0 + Dot(lambdas, sensitivities);
}

std::optional<Vector> PureFactorPortfolio(const Matrix& b, std::size_t k) {
  INVESTM_CHECK(k < b.cols(), "factor index out of range");
  // Minimum-norm w with A'w = target: w = A (A'A)^-1 target.
  const Matrix a = WithOnes(b);
  const Matrix at = a.Transpose();
  Vector target(a.cols(), 0.0);
  target[0] = 1.0;
  target[k + 1] = 1.0;
  const auto y = Solve(at * a, target);
  if (!y.has_value()) return std::nullopt;
  return a * *y;
}

}  // namespace investm::ch11
