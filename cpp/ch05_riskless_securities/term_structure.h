// Chapter 5, The Valuation of Riskless Securities.
// Sections 5.3 (spot rates), 5.4 (discount factors), 5.5 (forward rates),
// 5.6 (forward rates and discount factors), 5.9 (yield curves), 5.10 (term
// structure theories) and 5.11 (holding-period return).
//
// Rates are annually compounded and maturities are whole years, which keeps
// every number checkable by hand. s_t is the t-year spot rate,
// d_t = 1 / (1 + s_t)^t the discount factor and f_{a,b} the forward rate
// from year a to year b.

#ifndef CPP_CH05_RISKLESS_SECURITIES_TERM_STRUCTURE_H_
#define CPP_CH05_RISKLESS_SECURITIES_TERM_STRUCTURE_H_

#include <optional>
#include <vector>

#include "cpp/ch05_riskless_securities/cash_flows.h"

namespace investm::ch05 {

class SpotCurve {
 public:
  // spot_rates[0] is the 1-year rate, spot_rates[1] the 2-year rate, etc.
  explicit SpotCurve(std::vector<double> spot_rates);

  static SpotCurve FromDiscountFactors(const std::vector<double>& factors);
  // (1 + s_t)^t = (1 + f_{0,1}) (1 + f_{1,2}) ... (1 + f_{t-1,t}).
  static SpotCurve FromForwards(const std::vector<double>& one_year_forwards);

  int max_maturity() const { return static_cast<int>(spot_.size()); }
  const std::vector<double>& rates() const { return spot_; }

  double Spot(int t) const;
  // d_t = (1 + s_t)^-t, with d_0 = 1.
  double DiscountFactor(int t) const;
  // (1 + f_{a,b})^(b - a) = d_a / d_b, for 0 <= a < b.
  double Forward(int a, int b) const;
  double OneYearForward(int t) const { return Forward(t - 1, t); }
  // Sum of CF_t * d_t. Cash flows must fall on whole years.
  double PresentValue(const CashFlows& flows) const;
  // Coupon rate that prices a t-year annual-pay bond at par:
  //   c = (1 - d_t) / (d_1 + ... + d_t).
  double ParYield(int t) const;

 private:
  std::vector<double> spot_;
};

// 5.3 Bootstrapping spot rates from annual-coupon bond prices. quotes[i] must
// mature in year i + 1. Solving one maturity at a time:
//   P_n = c_n F (d_1 + ... + d_{n-1}) + (1 + c_n) F d_n.
// Returns nullopt if the prices imply a non-positive discount factor.
struct BondQuote {
  double face;
  double coupon_rate;  // Annual coupon, paid once a year.
  double price;
};
std::optional<SpotCurve> Bootstrap(const std::vector<BondQuote>& quotes);

// 5.10.1 Unbiased expectations: forward rates equal the market's expected
// future one-year spot rates.
std::vector<double> ExpectedFutureOneYearRates(const SpotCurve& curve);

// 5.10.2 Liquidity preference: f_{t-1,t} = E[future one-year rate] + L_t.
SpotCurve LiquidityPreferenceCurve(const std::vector<double>& expected,
                                   const std::vector<double>& premiums);
std::vector<double> ImpliedLiquidityPremiums(
    const SpotCurve& curve, const std::vector<double>& expected);

// 5.11 Holding-period return.
double HoldingPeriodReturn(double buy_price, double sell_price,
                           double coupons = 0.0);

// One-year HPR of an n-year zero bought off `today` and sold a year later,
// with n - 1 years left, off `next_year`.
double OneYearHprOfZero(const SpotCurve& today, const SpotCurve& next_year,
                        int n);

// The curve one year from now if today's forward rates come true.
SpotCurve RollForwardOneYear(const SpotCurve& curve);

}  // namespace investm::ch05

#endif  // CPP_CH05_RISKLESS_SECURITIES_TERM_STRUCTURE_H_
