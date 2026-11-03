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

AffineSimplexProjector::AffineSimplexProjector(std::span<const double> a)
    : a_(a.begin(), a.end()) {
  INVESTM_CHECK(!a_.empty(), "empty constraint vector");
  INVESTM_CHECK(
      std::any_of(a_.begin(), a_.end(), [](double x) { return x > 0.0; }),
      "a'z = 1 with z >= 0 needs some a_i > 0");
  breakpoints_.reserve(a_.size());
}

void AffineSimplexProjector::Project(std::span<const double> v,
                                     std::span<double> out) {
  const std::size_t n = a_.size();
  INVESTM_CHECK(v.size() == n && out.size() == n, "size mismatch");
  // g(theta) = sum_i a_i max(0, v_i - theta a_i) is continuous and
  // non-increasing. Between breakpoints the active set is fixed and
  // g = S1 - theta S2 with S1 = sum a_i v_i, S2 = sum a_i^2 over it. At
  // theta = -inf the active set is {a_i > 0}; crossing b_i = v_i / a_i
  // removes i if a_i > 0 and adds it if a_i < 0.
  breakpoints_.clear();
  double s1 = 0.0;
  double s2 = 0.0;
  for (std::size_t i = 0; i < n; ++i) {
    if (a_[i] == 0.0) continue;
    breakpoints_.emplace_back(v[i] / a_[i], i);
    if (a_[i] > 0.0) {
      s1 += a_[i] * v[i];
      s2 += a_[i] * a_[i];
    }
  }
  std::sort(breakpoints_.begin(), breakpoints_.end());
  double theta = 0.0;
  bool found = false;
  for (const auto& [b, i] : breakpoints_) {
    if (s1 - b * s2 <= 1.0) {  // Root lies left of b, in a linear piece.
      theta = s2 > 0.0 ? (s1 - 1.0) / s2 : b;
      found = true;
      break;
    }
    const double sign = a_[i] > 0.0 ? -1.0 : 1.0;
    s1 += sign * a_[i] * v[i];
    s2 += sign * a_[i] * a_[i];
  }
  if (!found) theta = (s1 - 1.0) / s2;  // s2 > 0: some a_i < 0 is active.
  for (std::size_t i = 0; i < n; ++i) {
    out[i] = std::max(0.0, v[i] - theta * a_[i]);
  }
}

int SolveAffineSimplexQp(const Matrix& cov, std::span<const double> c,
                         std::span<const double> a, double lipschitz,
                         std::span<double> z, QpOptions options) {
  const std::size_t n = c.size();
  INVESTM_CHECK(
      cov.rows() == n && cov.cols() == n && a.size() == n && z.size() == n,
      "dimension mismatch");
  const double step = 1.0 / std::max(lipschitz, 1e-300);
  AffineSimplexProjector projector(a);
  // All buffers live outside the loop: each iteration is one matrix-vector
  // product, one projection and a few O(n) passes.
  Vector w(n);
  Vector y(n);
  Vector sy(n);
  Vector trial(n);
  Vector w_next(n);
  projector.Project(z, w);
  y = w;
  double t = 1.0;
  int it = 0;
  while (it < options.max_iterations) {
    ++it;
    MultiplyInto(cov, y, sy);
    for (std::size_t i = 0; i < n; ++i) {
      trial[i] = y[i] - step * (2.0 * sy[i] - c[i]);
    }
    projector.Project(trial, w_next);
    // Gradient-based adaptive restart (O'Donoghue and Candes, 2015): when
    // the momentum points uphill, drop it. This restores linear convergence
    // on strongly convex problems instead of FISTA's oscillation.
    double uphill = 0.0;
    double change = 0.0;
    for (std::size_t i = 0; i < n; ++i) {
      uphill += (y[i] - w_next[i]) * (w_next[i] - w[i]);
      change = std::max(change, std::fabs(w_next[i] - w[i]));
    }
    double t_next = 0.5 * (1.0 + std::sqrt(1.0 + 4.0 * t * t));
    double beta = (t - 1.0) / t_next;
    if (uphill > 0.0) {
      t_next = 1.0;
      beta = 0.0;
    }
    for (std::size_t i = 0; i < n; ++i) {
      y[i] = w_next[i] + beta * (w_next[i] - w[i]);
    }
    std::swap(w, w_next);
    t = t_next;
    if (change < options.tolerance) break;
  }
  std::copy(w.begin(), w.end(), z.begin());
  return it;
}

PortfolioPoint OptimalNoShortSales(std::span<const double> mu,
                                   const Matrix& cov, double tau,
                                   QpOptions options) {
  const std::vector<PortfolioPoint> one = EfficientSetNoShortSales(
      mu, cov, std::span<const double>(&tau, 1), options);
  return one.front();
}

std::vector<PortfolioPoint> EfficientSetNoShortSales(
    std::span<const double> mu, const Matrix& cov, std::span<const double> taus,
    QpOptions options) {
  const std::size_t n = mu.size();
  INVESTM_CHECK(n > 0, "no securities");
  INVESTM_CHECK(cov.rows() == n && cov.cols() == n, "dimension mismatch");
  // Minimize f(w) = w'Sw - tau mu'w; grad = 2 S w - tau mu, Lipschitz
  // 2 lambda_max(S), the same for every tau.
  const double lipschitz = 2.0 * LargestEigenvalue(cov);
  const Vector ones(n, 1.0);
  Vector c(n);
  Vector w(n, 1.0 / static_cast<double>(n));
  std::vector<PortfolioPoint> points;
  points.reserve(taus.size());
  for (const double tau : taus) {
    INVESTM_CHECK(tau >= 0.0, "risk tolerance must be non-negative");
    for (std::size_t i = 0; i < n; ++i) c[i] = tau * mu[i];
    SolveAffineSimplexQp(cov, c, ones, lipschitz, w, options);
    points.push_back(Evaluate(w, mu, cov));
  }
  return points;
}

double TwoAssetMinVarianceWeight(double s1, double s2, double rho) {
  const double denom = s1 * s1 + s2 * s2 - 2.0 * rho * s1 * s2;
  INVESTM_CHECK(denom > 0.0, "identical riskless combination");
  return (s2 * s2 - rho * s1 * s2) / denom;
}

}  // namespace investm::ch07
