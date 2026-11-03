// Options and futures: equations 55 to 60. `rate` is the continuously
// compounded risk-free rate and `years` the time to expiry.

#ifndef INVESTM_FORMULAS_DERIVATIVES_H_
#define INVESTM_FORMULAS_DERIVATIVES_H_

namespace investm::formulas {

// Eq. 55: IV_C = max(0, P_S - E).
double CallIntrinsicValue(double stock_price, double strike);

// Eq. 55: IV_P = max(0, E - P_S).
double PutIntrinsicValue(double stock_price, double strike);

// Eq. 56 solved for the put: P_P = P_C + E / e^(RT) - P_S.
double PutFromParity(double call_price, double stock_price, double strike,
                     double rate, double years);

// Eq. 56 solved for the call: P_C = P_P + P_S - E / e^(RT).
double CallFromParity(double put_price, double stock_price, double strike,
                      double rate, double years);

// Standard normal cumulative distribution N(x).
double NormalCdf(double x);

// Eq. 57: d1 = [ln(P_S / E) + (R + 0.5 sigma^2) T] / (sigma sqrt(T)).
double BlackScholesD1(double stock_price, double strike, double rate,
                      double years, double volatility);

// Eq. 57: d2 = d1 - sigma sqrt(T).
double BlackScholesD2(double stock_price, double strike, double rate,
                      double years, double volatility);

// Eq. 57: V_C = N(d1) P_S - E / e^(RT) N(d2).
double BlackScholesCall(double stock_price, double strike, double rate,
                        double years, double volatility);

// Eq. 58: P_P = E / e^(RT) N(-d2) - P_S N(-d1).
double BlackScholesPut(double stock_price, double strike, double rate,
                       double years, double volatility);

// Eq. 59: P_f = P_s + I - B + C.
double FuturesFairValue(double spot_price, double interest, double benefits,
                        double costs);

// Eq. 60: P_f = P_s + R P_s - y P_s.
double IndexFuturesFairValue(double spot_price, double rate,
                             double dividend_yield);

}  // namespace investm::formulas

#endif  // INVESTM_FORMULAS_DERIVATIVES_H_
