#include "cpp/ch07_portfolio_analysis/efficient_set.h"

#include <algorithm>
#include <cmath>
#include <functional>
#include <utility>
#include <vector>

#include "cpp/core/check.h"

namespace investm::ch07 {

PortfolioPoint Evaluate(std::span<const double> weights,
                        std::span<const double> mu, const Matrix& cov) {
  return {Vector(weights.begin(), weights.end()), Dot(weights, mu),
          std::sqrt(std::max(0.0, QuadraticForm(weights, cov)))};
}

std::optional<UnconstrainedFrontier> UnconstrainedFrontier::Create(Vector mu,
                                                                   Matrix cov) {
  INVESTM_CHECK(cov.rows() == mu.size() && cov.cols() == mu.size(),
                "dimension mismatch");
  auto inv = Inverse(cov);
  if (!inv.has_value()) return std::nullopt;
  UnconstrainedFrontier f(std::move(mu), std::move(cov), std::move(*inv));
  if (f.d_ <= 1e-14 * f.a_ * f.c_) return std::nullopt;
  return f;
}

UnconstrainedFrontier::UnconstrainedFrontier(Vector mu, Matrix cov,
                                             Matrix cov_inv)
    : mu_(std::move(mu)), cov_(std::move(cov)) {
  const Vector ones(mu_.size(), 1.0);
  inv_ones_ = cov_inv * ones;
  inv_mu_ = cov_inv * mu_;
  a_ = Dot(ones, inv_ones_);
  b_ = Dot(ones, inv_mu_);
  c_ = Dot(mu_, inv_mu_);
  d_ = a_ * c_ - b_ * b_;
}

PortfolioPoint UnconstrainedFrontier::MinimumVariance() const {
  Vector w(inv_ones_);
  for (double& x : w) x /= a_;
  return Evaluate(w, mu_, cov_);
}

PortfolioPoint UnconstrainedFrontier::ForTargetReturn(double target) const {
  // w = S^-1 (lambda 1 + gamma mu), from the two Lagrange conditions.
  const double lambda = (c_ - b_ * target) / d_;
  const double gamma = (a_ * target - b_) / d_;
  Vector w(mu_.size());
  for (std::size_t i = 0; i < w.size(); ++i) {
    w[i] = lambda * inv_ones_[i] + gamma * inv_mu_[i];
  }
  return Evaluate(w, mu_, cov_);
}

double UnconstrainedFrontier::VarianceAt(double target) const {
  return (a_ * target * target - 2.0 * b_ * target + c_) / d_;
}

PortfolioPoint UnconstrainedFrontier::Optimal(double tau) const {
  INVESTM_CHECK(tau >= 0.0, "risk tolerance must be non-negative");
  // Maximize tau mu'w - w'Sw s.t. 1'w = 1:
  //   w = (tau / 2) S^-1 mu + eta S^-1 1, eta = (1 - (tau / 2) B) / A.
  const double eta = (1.0 - 0.5 * tau * b_) / a_;
  Vector w(mu_.size());
  for (std::size_t i = 0; i < w.size(); ++i) {
    w[i] = 0.5 * tau * inv_mu_[i] + eta * inv_ones_[i];
  }
  return Evaluate(w, mu_, cov_);
}

Vector ProjectOntoSimplex(std::span<const double> v) {
  INVESTM_CHECK(!v.empty(), "empty vector");
  // Sort-based algorithm (Held, Wolfe and Crowder; Duchi et al.).
  Vector u(v.begin(), v.end());
  std::sort(u.begin(), u.end(), std::greater<>());
  double cumulative = 0.0;
  double theta = 0.0;
  for (std::size_t k = 0; k < u.size(); ++k) {
    cumulative += u[k];
    const double t = (cumulative - 1.0) / static_cast<double>(k + 1);
    if (u[k] - t > 0.0) theta = t;
  }
  Vector w(v.size());
  for (std::size_t i = 0; i < v.size(); ++i) w[i] = std::max(0.0, v[i] - theta);
  return w;
}

PortfolioPoint OptimalNoShortSales(std::span<const double> mu,
                                   const Matrix& cov, double tau,
                                   QpOptions options) {
  const std::size_t n = mu.size();
  INVESTM_CHECK(cov.rows() == n && cov.cols() == n, "dimension mismatch");
  INVESTM_CHECK(tau >= 0.0, "risk tolerance must be non-negative");
  // Minimize f(w) = w'Sw - tau mu'w; grad = 2 S w - tau mu, Lipschitz 2 l_max.
  const double l_max = EigenSymmetric(cov).values.front();
  const double step = 1.0 / (2.0 * std::max(l_max, 1e-300));
  Vector w(n, 1.0 / static_cast<double>(n));
  Vector y = w;
  Vector trial(n);
  double t = 1.0;
  for (int it = 0; it < options.max_iterations; ++it) {
    const Vector sy = cov * y;
    for (std::size_t i = 0; i < n; ++i) {
      trial[i] = y[i] - step * (2.0 * sy[i] - tau * mu[i]);
    }
    const Vector w_next = ProjectOntoSimplex(trial);
    const double t_next = 0.5 * (1.0 + std::sqrt(1.0 + 4.0 * t * t));
    double change = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
      y[i] = w_next[i] + (t - 1.0) / t_next * (w_next[i] - w[i]);
      change = std::max(change, std::fabs(w_next[i] - w[i]));
    }
    w = w_next;
    t = t_next;
    if (change < options.tolerance) break;
  }
  return Evaluate(w, mu, cov);
}

std::vector<PortfolioPoint> EfficientSetNoShortSales(
    std::span<const double> mu, const Matrix& cov, std::span<const double> taus,
    QpOptions options) {
  std::vector<PortfolioPoint> points;
  points.reserve(taus.size());
  for (double tau : taus) {
    points.push_back(OptimalNoShortSales(mu, cov, tau, options));
  }
  return points;
}

double TwoAssetMinVarianceWeight(double s1, double s2, double rho) {
  const double denom = s1 * s1 + s2 * s2 - 2.0 * rho * s1 * s2;
  INVESTM_CHECK(denom > 0.0, "identical riskless combination");
  return (s2 * s2 - rho * s1 * s2) / denom;
}

}  // namespace investm::ch07
