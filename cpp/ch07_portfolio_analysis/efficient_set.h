// Chapter 7, Portfolio Analysis.
// Sections 7.1 (the efficient set theorem, the feasible set, selection of the
// optimal portfolio), 7.2 (concavity of the efficient set) and Appendix A
// (the Markowitz model: composition and location of the efficient set and of
// the optimal portfolio).
//
// Two versions of the problem are solved:
//  * Short sales allowed. The efficient set then has a closed form in terms
//    of the scalars A = 1'S^-1 1, B = 1'S^-1 mu, C = mu'S^-1 mu.
//  * No short sales (all weights >= 0), the case the book's Markowitz
//    discussion emphasizes. Solved as a convex quadratic program by
//    accelerated projected gradient on the simplex.

#ifndef CPP_CH07_PORTFOLIO_ANALYSIS_EFFICIENT_SET_H_
#define CPP_CH07_PORTFOLIO_ANALYSIS_EFFICIENT_SET_H_

#include <optional>
#include <span>
#include <vector>

#include "cpp/core/matrix.h"

namespace investm::ch07 {

struct PortfolioPoint {
  Vector weights;
  double expected_return = 0.0;
  double std_dev = 0.0;
};

PortfolioPoint Evaluate(std::span<const double> weights,
                        std::span<const double> mu, const Matrix& cov);

// ---------------------------------------------------------------------------
// Short sales allowed.
// ---------------------------------------------------------------------------
class UnconstrainedFrontier {
 public:
  // Returns nullopt if the covariance matrix is singular or all expected
  // returns are equal (the frontier degenerates to a point).
  static std::optional<UnconstrainedFrontier> Create(Vector mu, Matrix cov);

  // The global minimum-variance portfolio: S^-1 1 / A.
  PortfolioPoint MinimumVariance() const;
  // The frontier portfolio with expected return `target`.
  PortfolioPoint ForTargetReturn(double target) const;
  // Frontier variance as a function of E: (A E^2 - 2 B E + C) / D.
  double VarianceAt(double target) const;
  // 7.1.3 The portfolio maximizing E - sigma^2 / tau.
  PortfolioPoint Optimal(double tau) const;

  double a() const { return a_; }
  double b() const { return b_; }
  double c() const { return c_; }

 private:
  UnconstrainedFrontier(Vector mu, Matrix cov, Matrix cov_inv);

  Vector mu_;
  Matrix cov_;
  Vector inv_ones_;  // S^-1 1
  Vector inv_mu_;    // S^-1 mu
  double a_ = 0.0;
  double b_ = 0.0;
  double c_ = 0.0;
  double d_ = 0.0;
};

// ---------------------------------------------------------------------------
// No short sales.
// ---------------------------------------------------------------------------

// Euclidean projection of v onto {w : w >= 0, sum w = 1}.
Vector ProjectOntoSimplex(std::span<const double> v);

struct QpOptions {
  int max_iterations = 20000;
  double tolerance = 1e-12;  // On the change in weights per iteration.
};

// Maximizes tau * mu'w - w'S w over the simplex (equivalent to maximizing
// E - sigma^2 / tau). tau = 0 gives the minimum-variance portfolio.
PortfolioPoint OptimalNoShortSales(std::span<const double> mu,
                                   const Matrix& cov, double tau,
                                   QpOptions options = {});

// Traces the no-short-sales efficient set by sweeping risk tolerance.
std::vector<PortfolioPoint> EfficientSetNoShortSales(
    std::span<const double> mu, const Matrix& cov, std::span<const double> taus,
    QpOptions options = {});

// ---------------------------------------------------------------------------
// 7.2 Two-security combinations.
// ---------------------------------------------------------------------------

// Weight in security 1 that minimizes variance:
//   x1 = (s2^2 - rho s1 s2) / (s1^2 + s2^2 - 2 rho s1 s2).
double TwoAssetMinVarianceWeight(double s1, double s2, double rho);

}  // namespace investm::ch07

#endif  // CPP_CH07_PORTFOLIO_ANALYSIS_EFFICIENT_SET_H_
