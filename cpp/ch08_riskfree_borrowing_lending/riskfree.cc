#include "cpp/ch08_riskfree_borrowing_lending/riskfree.h"

#include <cmath>
#include <numeric>

#include "cpp/core/check.h"

namespace investm::ch08 {

Combination CombineWithRiskfree(double rf, double risky_expected,
                                double risky_std_dev, double x_risky) {
  return {(1.0 - x_risky) * rf + x_risky * risky_expected,
          std::fabs(x_risky) * risky_std_dev};
}

double SharpeRatio(double expected_return, double std_dev, double rf) {
  INVESTM_CHECK(std_dev > 0.0, "standard deviation must be positive");
  return (expected_return - rf) / std_dev;
}

std::optional<ch07::PortfolioPoint> TangencyPortfolio(
    std::span<const double> mu, const Matrix& cov, double rf) {
  const auto inv = Inverse(cov);
  if (!inv.has_value()) return std::nullopt;
  Vector excess(mu.begin(), mu.end());
  for (double& e : excess) e -= rf;
  Vector z = *inv * excess;
  const double total = std::accumulate(z.begin(), z.end(), 0.0);
  // total = B - A rf, positive exactly when rf < B / A.
  if (total <= 0.0) return std::nullopt;
  for (double& w : z) w /= total;
  return ch07::Evaluate(z, mu, cov);
}

ch07::PortfolioPoint TangencyPortfolioNoShortSales(std::span<const double> mu,
                                                   const Matrix& cov,
                                                   double rf) {
  auto sharpe_at = [&](double log_tau, ch07::PortfolioPoint* out) {
    const ch07::PortfolioPoint p =
        ch07::OptimalNoShortSales(mu, cov, std::exp(log_tau));
    if (out != nullptr) *out = p;
    return p.std_dev > 0.0 ? (p.expected_return - rf) / p.std_dev : -1e300;
  };
  // Search log(tau) in [-12, 6]: from (almost) minimum variance to the top
  // end point of the efficient set.
  constexpr double kInvPhi = 0.6180339887498949;
  double lo = -12.0;
  double hi = 6.0;
  double x1 = hi - kInvPhi * (hi - lo);
  double x2 = lo + kInvPhi * (hi - lo);
  double f1 = sharpe_at(x1, nullptr);
  double f2 = sharpe_at(x2, nullptr);
  for (int i = 0; i < 80 && hi - lo > 1e-7; ++i) {
    if (f1 < f2) {
      lo = x1;
      x1 = x2;
      f1 = f2;
      x2 = lo + kInvPhi * (hi - lo);
      f2 = sharpe_at(x2, nullptr);
    } else {
      hi = x2;
      x2 = x1;
      f2 = f1;
      x1 = hi - kInvPhi * (hi - lo);
      f1 = sharpe_at(x1, nullptr);
    }
  }
  ch07::PortfolioPoint best;
  sharpe_at(0.5 * (lo + hi), &best);
  return best;
}

double OptimalFractionInTangency(const ch07::PortfolioPoint& tangency,
                                 double rf, double tau) {
  INVESTM_CHECK(tangency.std_dev > 0.0, "tangency portfolio has no risk");
  return tau * (tangency.expected_return - rf) /
         (2.0 * tangency.std_dev * tangency.std_dev);
}

std::optional<OptimalChoice> OptimalWithDifferentRates(
    std::span<const double> mu, const Matrix& cov, double r_lend,
    double r_borrow, double tau) {
  INVESTM_CHECK(r_lend <= r_borrow, "lending rate must not exceed borrowing");
  const auto t_lend = TangencyPortfolio(mu, cov, r_lend);
  const auto t_borrow = TangencyPortfolio(mu, cov, r_borrow);
  if (!t_lend.has_value()) return std::nullopt;

  const double x_lend = OptimalFractionInTangency(*t_lend, r_lend, tau);
  if (x_lend <= 1.0) {
    const Combination c = CombineWithRiskfree(r_lend, t_lend->expected_return,
                                              t_lend->std_dev, x_lend);
    return OptimalChoice{Region::kLend, x_lend, c.expected_return, c.std_dev,
                         t_lend->weights};
  }
  // With r_borrow at or above the minimum-variance return there is no
  // borrowing tangency and borrowing is never worthwhile.
  const double x_borrow =
      t_borrow.has_value() ? OptimalFractionInTangency(*t_borrow, r_borrow, tau)
                           : 0.0;
  if (x_borrow >= 1.0) {
    const Combination c = CombineWithRiskfree(
        r_borrow, t_borrow->expected_return, t_borrow->std_dev, x_borrow);
    return OptimalChoice{Region::kBorrow, x_borrow, c.expected_return,
                         c.std_dev, t_borrow->weights};
  }
  const auto frontier =
      ch07::UnconstrainedFrontier::Create(Vector(mu.begin(), mu.end()), cov);
  if (!frontier.has_value()) return std::nullopt;
  const ch07::PortfolioPoint p = frontier->Optimal(tau);
  return OptimalChoice{Region::kRiskyOnly, 1.0, p.expected_return, p.std_dev,
                       p.weights};
}

}  // namespace investm::ch08
