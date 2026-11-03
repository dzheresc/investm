// Chapter 5, The Valuation of Riskless Securities.
// Section 5.2: yield-to-maturity, the single discount rate that equates the
// present value of a bond's promised cash flows with its market price.

#ifndef CPP_CH05_RISKLESS_SECURITIES_CASH_FLOWS_H_
#define CPP_CH05_RISKLESS_SECURITIES_CASH_FLOWS_H_

#include <optional>
#include <vector>

namespace investm::ch05 {

struct CashFlow {
  double time;  // Years from today.
  double amount;
};

using CashFlows = std::vector<CashFlow>;

// Promised cash flows of a level-coupon bullet bond. `years` must be a whole
// number of coupon periods; coupon_rate * face is paid per year in
// `frequency` instalments.
CashFlows CouponBond(double face, double coupon_rate, double years,
                     int frequency = 1);
CashFlows ZeroCoupon(double face, double years);

// Present value at a flat yield quoted with `frequency` compounding.
double PresentValueAtYield(const CashFlows& flows, double yield,
                           int frequency = 1);
// Present value at a flat continuously compounded yield.
double PresentValueContinuous(const CashFlows& flows, double yield);

// 5.2.1 Yield-to-maturity by root finding. Returns nullopt when the price is
// not attainable by any yield above -100%.
std::optional<double> YieldToMaturity(const CashFlows& flows, double price,
                                      int frequency = 1);

// Internal rate of return (annual compounding) of a stream that includes the
// time-0 outlay. Searches [lo, hi].
std::optional<double> InternalRateOfReturn(const CashFlows& flows,
                                           double lo = -0.99, double hi = 1.0);

}  // namespace investm::ch05

#endif  // CPP_CH05_RISKLESS_SECURITIES_CASH_FLOWS_H_
