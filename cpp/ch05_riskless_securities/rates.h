// Chapter 5, The Valuation of Riskless Securities.
// Section 5.1 (nominal versus real rates), 5.7 (compounding), 5.8 (the bank
// discount method) and the appendix on continuous compounding.

#ifndef CPP_CH05_RISKLESS_SECURITIES_RATES_H_
#define CPP_CH05_RISKLESS_SECURITIES_RATES_H_

namespace investm::ch05 {

// 5.1 Fisher relation: (1 + nominal) = (1 + real) * (1 + inflation).
double RealRate(double nominal, double inflation);
double NominalRate(double real, double inflation);
// The common approximation real = nominal - inflation.
inline double RealRateApprox(double nominal, double inflation) {
  return nominal - inflation;
}

// 5.7 A quoted annual rate (APR) compounded m times a year.
double EffectiveAnnualRate(double apr, int m);
double AprFromEffective(double effective, int m);
// The same rate quoted with m2 compounding periods instead of m1.
double ConvertCompounding(double apr, int m1, int m2);
double FutureValue(double present_value, double apr, int m, double years);

// Appendix: continuous compounding, the limit as m goes to infinity.
double EffectiveFromContinuous(double continuous_rate);
double ContinuousFromEffective(double effective);
double FutureValueContinuous(double present_value, double continuous_rate,
                             double years);

// 5.8 Treasury bills quoted on a bank discount basis. With discount yield d,
// `days` to maturity and face value F:  price = F * (1 - d * days / 360).
// The discount yield understates the return because it uses face value, not
// price, as the base and a 360-day year.
double BillPriceFromDiscount(double face, double discount_yield, int days);
double BankDiscountYield(double face, double price, int days);
// Bond-equivalent yield, for bills with at most half a year to maturity.
double BondEquivalentYield(double face, double price, int days);
// Money-market (CD-equivalent) yield: price base, 360-day year.
double MoneyMarketYield(double face, double price, int days);
double BillEffectiveAnnualYield(double face, double price, int days);

}  // namespace investm::ch05

#endif  // CPP_CH05_RISKLESS_SECURITIES_RATES_H_
