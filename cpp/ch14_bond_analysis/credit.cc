#include "cpp/ch14_bond_analysis/credit.h"

#include <cmath>
#include <utility>
#include <vector>

#include "cpp/core/check.h"

namespace investm::ch14 {
namespace {

Vector ColumnMeans(const Matrix& m) {
  Vector mean(m.cols(), 0.0);
  for (std::size_t r = 0; r < m.rows(); ++r) {
    for (std::size_t c = 0; c < m.cols(); ++c) mean[c] += m(r, c);
  }
  for (double& x : mean) x /= static_cast<double>(m.rows());
  return mean;
}

}  // namespace

ch05::CashFlows ExpectedCashFlows(double face, double coupon_rate, int years,
                                  double default_prob, double recovery) {
  INVESTM_CHECK(years > 0, "maturity must be positive");
  INVESTM_CHECK(default_prob >= 0.0 && default_prob <= 1.0,
                "probability in [0, 1]");
  INVESTM_CHECK(recovery >= 0.0 && recovery <= 1.0, "recovery in [0, 1]");
  ch05::CashFlows flows;
  double survive = 1.0;  // Probability of reaching the start of year t.
  for (int t = 1; t <= years; ++t) {
    const double promised = face * coupon_rate + (t == years ? face : 0.0);
    const double pays = survive * (1.0 - default_prob) * promised;
    const double recovered = survive * default_prob * recovery * face;
    flows.push_back({static_cast<double>(t), pays + recovered});
    survive *= 1.0 - default_prob;
  }
  return flows;
}

std::optional<YieldDecomposition> DecomposeYield(
    double face, double coupon_rate, int years, double price,
    double default_prob, double recovery, double riskfree) {
  const auto promised =
      ch05::YieldToMaturity(ch05::CouponBond(face, coupon_rate, years), price);
  const auto expected = ch05::YieldToMaturity(
      ExpectedCashFlows(face, coupon_rate, years, default_prob, recovery),
      price);
  if (!promised || !expected) return std::nullopt;
  return YieldDecomposition{*promised, *expected, *promised - *expected,
                            *expected - riskfree};
}

double ImpliedDefaultProbability(double risky_yield, double riskfree_yield,
                                 double recovery) {
  INVESTM_CHECK(recovery >= 0.0 && recovery < 1.0, "recovery in [0, 1)");
  return (1.0 - (1.0 + riskfree_yield) / (1.0 + risky_yield)) /
         (1.0 - recovery);
}

std::optional<LinearDiscriminant> LinearDiscriminant::Fit(
    const Matrix& group_a, const Matrix& group_b) {
  INVESTM_CHECK(group_a.cols() == group_b.cols(), "ratio count mismatch");
  INVESTM_CHECK(group_a.rows() >= 2 && group_b.rows() >= 2,
                "need two firms per group");
  const std::size_t k = group_a.cols();
  const Vector ma = ColumnMeans(group_a);
  const Vector mb = ColumnMeans(group_b);
  // Pooled within-group covariance.
  Matrix s(k, k);
  for (const auto* g : {&group_a, &group_b}) {
    const Vector& m = g == &group_a ? ma : mb;
    for (std::size_t r = 0; r < g->rows(); ++r) {
      for (std::size_t i = 0; i < k; ++i) {
        for (std::size_t j = 0; j < k; ++j) {
          s(i, j) += ((*g)(r, i) - m[i]) * ((*g)(r, j) - m[j]);
        }
      }
    }
  }
  const double dof = static_cast<double>(group_a.rows() + group_b.rows() - 2);
  s = (1.0 / dof) * s;
  Vector diff(k);
  Vector mid(k);
  for (std::size_t i = 0; i < k; ++i) {
    diff[i] = ma[i] - mb[i];
    mid[i] = 0.5 * (ma[i] + mb[i]);
  }
  auto w = Solve(s, diff);
  if (!w) return std::nullopt;
  const double cutoff = Dot(*w, mid);
  return LinearDiscriminant(std::move(*w), cutoff);
}

double LinearDiscriminant::Score(std::span<const double> x) const {
  INVESTM_CHECK(x.size() == weights_.size(), "ratio count mismatch");
  return Dot(weights_, x);
}

}  // namespace investm::ch14
