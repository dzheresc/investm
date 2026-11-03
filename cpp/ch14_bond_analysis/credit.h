// Chapter 14, Bond Analysis.
// Section 14.4-14.6: default risk. Promised versus expected yield, the
// default premium, default probabilities implied by yield spreads, and a
// linear discriminant credit score built from financial ratios.

#ifndef CPP_CH14_BOND_ANALYSIS_CREDIT_H_
#define CPP_CH14_BOND_ANALYSIS_CREDIT_H_

#include <optional>
#include <span>
#include <utility>

#include "cpp/ch05_riskless_securities/cash_flows.h"
#include "cpp/core/matrix.h"

namespace investm::ch14 {

// Expected cash flows of an annual-pay bond that defaults each year with
// probability `default_prob` (given survival so far). On default in year t
// the holder recovers `recovery` x face at t and nothing afterwards.
ch05::CashFlows ExpectedCashFlows(double face, double coupon_rate, int years,
                                  double default_prob, double recovery);

struct YieldDecomposition {
  double promised_yield;   // IRR of promised flows at the market price.
  double expected_yield;   // IRR of expected flows at the market price.
  double default_premium;  // promised - expected.
  double risk_premium;     // expected - riskfree.
};
std::optional<YieldDecomposition> DecomposeYield(
    double face, double coupon_rate, int years, double price,
    double default_prob, double recovery, double riskfree);

// One-period default probability that makes a risky zero priced at
// `risky_yield` fairly priced against `riskfree_yield` for a risk-neutral
// investor: (1 + r) = (1 + y)(1 - p (1 - R)).
double ImpliedDefaultProbability(double risky_yield, double riskfree_yield,
                                 double recovery);

// 14.6 Fisher linear discriminant between two groups of firms (rows are
// firms, columns are ratios). Score(x) = w . x; firms scoring above `cutoff`
// look like `group_a`.
class LinearDiscriminant {
 public:
  static std::optional<LinearDiscriminant> Fit(const Matrix& group_a,
                                               const Matrix& group_b);
  double Score(std::span<const double> x) const;
  bool IsGroupA(std::span<const double> x) const { return Score(x) > cutoff_; }
  const Vector& weights() const { return weights_; }
  double cutoff() const { return cutoff_; }

 private:
  LinearDiscriminant(Vector weights, double cutoff)
      : weights_(std::move(weights)), cutoff_(cutoff) {}
  Vector weights_;
  double cutoff_;
};

}  // namespace investm::ch14

#endif  // CPP_CH14_BOND_ANALYSIS_CREDIT_H_
