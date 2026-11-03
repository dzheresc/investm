// Taxes, growth, real returns and bonds: equations 25 to 34.

#ifndef INVESTM_FORMULAS_FIXED_INCOME_H_
#define INVESTM_FORMULAS_FIXED_INCOME_H_

#include <span>

namespace investm::formulas {

// Eq. 25: t = 1 - tax-exempt yield / taxable yield.
double BreakEvenTaxRate(double tax_exempt_yield, double taxable_yield);

// Eq. 26: g = (C_e / C_b)^(1/y) - 1.
double GeometricMeanGrowthRate(double beginning_value, double ending_value,
                               double years);

// Eq. 27, exact: RR = (1 + NR) / (1 + CCL) - 1.
double RealReturn(double nominal_return, double inflation);

// Eq. 27, approximation: RR ~ NR - CCL.
double ApproxRealReturn(double nominal_return, double inflation);

// Eq. 28: P_b = sum C_t / (1+y)^t + M / (1+y)^N. `coupons` holds C_1..C_N
// and must not be empty; the face value M is paid at N.
double BondPrice(std::span<const double> coupons, double face_value,
                 double ytm);

// Eq. 28 solved for y by bisection on [low, high]. Returns NaN when the
// price is not bracketed by the interval.
double YieldToMaturity(double price, std::span<const double> coupons,
                       double face_value, double low = -0.99,
                       double high = 10.0, double tolerance = 1e-12);

// Eq. 29: d_t = 1 / (1 + s_t)^t.
double DiscountFactor(double spot_rate, double years);

// Eq. 30: f_{t-1,t} = (1 + s_t)^t / (1 + s_{t-1})^(t-1) - 1.
double ForwardRate(double spot_prev, double spot_t, int t);

// Eq. 31: D = sum PV(C_t) * t / P_0, with present values at `ytm`.
// `cash_flows` holds C_1..C_T, principal included in the last one.
double MacaulayDuration(std::span<const double> cash_flows, double ytm);

// Eq. 32: D_m = D / (1 + y).
double ModifiedDuration(double duration, double ytm);

// Eq. 33: D_e = (P+ - P-) / (2 P dy). P+ is the price after the yield falls
// by dy, P- the price after it rises by dy.
double EffectiveDuration(double price_yield_down, double price_yield_up,
                         double price, double yield_change);

// Eq. 34: dP/P = -D dy / (1 + y).
double PriceChangeFromDuration(double duration, double ytm,
                               double yield_change);

// Eq. 34: dP/P = -D_m dy.
double PriceChangeFromModifiedDuration(double modified_duration,
                                       double yield_change);

}  // namespace investm::formulas

#endif  // INVESTM_FORMULAS_FIXED_INCOME_H_
