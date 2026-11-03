// Chapter 25, International Investing.
// Returns on foreign investments in domestic currency, currency risk,
// interest rate parity, hedged returns, purchasing power parity and the
// diversification benefit of adding foreign markets.

#ifndef CPP_CH25_INTERNATIONAL_INTERNATIONAL_H_
#define CPP_CH25_INTERNATIONAL_INTERNATIONAL_H_

#include <span>
#include <vector>

namespace investm::ch25 {

// Exchange rates are domestic currency per unit of foreign currency.

// 25.3 Currency return: S_1 / S_0 - 1.
double CurrencyReturn(double spot0, double spot1);
// Domestic-currency return: (1 + r_foreign)(1 + e) - 1.
double DomesticReturn(double foreign_return, double currency_return);
// Element-wise over a history.
std::vector<double> DomesticReturns(std::span<const double> foreign_returns,
                                    std::span<const double> currency_returns);
// Approximate variance of the domestic return (ignoring the cross term):
// var(r) + var(e) + 2 cov(r, e).
double DomesticReturnVariance(double foreign_std_dev, double currency_std_dev,
                              double correlation);

// 25.4 Covered interest parity forward rate for `years`, with annually
// compounded rates: F = S (1 + r_d)^T / (1 + r_f)^T.
double ParityForward(double spot, double domestic_rate, double foreign_rate,
                     double years);
// Forward premium (F / S - 1).
double ForwardPremium(double spot, double forward);
// Domestic return when the expected end value of a foreign investment is
// sold forward: (1 + r_foreign)(F / S_0) - 1, plus the unhedged surprise
// (r_foreign - hedged_foreign_return) translated at the realized spot.
double HedgedReturn(double foreign_return, double hedged_foreign_return,
                    double spot0, double forward, double spot1);

// 25.5 Relative purchasing power parity: expected currency change
// (1 + i_d) / (1 + i_f) - 1, and the real exchange rate S P_f / P_d.
double PppExpectedChange(double domestic_inflation, double foreign_inflation);
double RealExchangeRate(double spot, double foreign_price_level,
                        double domestic_price_level);

// 25.6 Standard deviation of a domestic/foreign mix with weight
// `foreign_weight` abroad, by correlation.
double TwoMarketStdDev(double foreign_weight, double domestic_std_dev,
                       double foreign_std_dev, double correlation);
// Weight abroad that minimizes the two-market standard deviation.
double MinimumRiskForeignWeight(double domestic_std_dev, double foreign_std_dev,
                                double correlation);

}  // namespace investm::ch25

#endif  // CPP_CH25_INTERNATIONAL_INTERNATIONAL_H_
