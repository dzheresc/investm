// Chapter 14, Bond Analysis.
// Section 14.2-14.3: bond pricing theorems, duration and convexity. Price
// sensitivity of a bond to its yield, and the effective (numerical) versions
// used for bonds whose cash flows depend on rates.

#ifndef CPP_CH14_BOND_ANALYSIS_DURATION_H_
#define CPP_CH14_BOND_ANALYSIS_DURATION_H_

#include <functional>

#include "cpp/ch05_riskless_securities/cash_flows.h"

namespace investm::ch14 {

using ch05::CashFlows;

// Macaulay duration in years: the present-value-weighted average time to
// each cash flow, discounting at a flat yield with `frequency` compounding.
double MacaulayDuration(const CashFlows& flows, double yield,
                        int frequency = 1);
// Modified duration D / (1 + y/f): -(1/P) dP/dy.
double ModifiedDuration(const CashFlows& flows, double yield,
                        int frequency = 1);
// Dollar duration -dP/dy and the price value of one basis point.
double DollarDuration(const CashFlows& flows, double yield, int frequency = 1);
double PriceValueOfBasisPoint(const CashFlows& flows, double yield,
                              int frequency = 1);
// Convexity (1/P) d^2P/dy^2, in years squared.
double Convexity(const CashFlows& flows, double yield, int frequency = 1);

// Second-order price change estimate for a yield change dy:
//   dP / P ~ -D_mod dy + 0.5 C dy^2.
double ApproxPriceChange(double price, double modified_duration,
                         double convexity, double dy);

// Duration of a level perpetuity: (1 + y) / y.
double PerpetuityDuration(double yield);

// Effective duration and convexity from a pricing function by central
// differences; works for bonds with embedded options.
using Pricer = std::function<double(double yield)>;
double EffectiveDuration(const Pricer& price, double yield, double dy = 1e-4);
double EffectiveConvexity(const Pricer& price, double yield, double dy = 1e-4);

}  // namespace investm::ch14

#endif  // CPP_CH14_BOND_ANALYSIS_DURATION_H_
