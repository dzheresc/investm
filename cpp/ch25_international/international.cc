#include "cpp/ch25_international/international.h"

#include <cmath>
#include <vector>

#include "cpp/core/check.h"

namespace investm::ch25 {

double CurrencyReturn(double spot0, double spot1) {
  INVESTM_CHECK(spot0 > 0.0 && spot1 > 0.0, "rates must be positive");
  return spot1 / spot0 - 1.0;
}

double DomesticReturn(double foreign_return, double currency_return) {
  return (1.0 + foreign_return) * (1.0 + currency_return) - 1.0;
}

std::vector<double> DomesticReturns(std::span<const double> foreign_returns,
                                    std::span<const double> currency_returns) {
  INVESTM_CHECK(foreign_returns.size() == currency_returns.size(),
                "size mismatch");
  std::vector<double> r(foreign_returns.size());
  for (std::size_t t = 0; t < r.size(); ++t) {
    r[t] = DomesticReturn(foreign_returns[t], currency_returns[t]);
  }
  return r;
}

double DomesticReturnVariance(double foreign_std_dev, double currency_std_dev,
                              double correlation) {
  return foreign_std_dev * foreign_std_dev +
         currency_std_dev * currency_std_dev +
         2.0 * correlation * foreign_std_dev * currency_std_dev;
}

double ParityForward(double spot, double domestic_rate, double foreign_rate,
                     double years) {
  INVESTM_CHECK(spot > 0.0 && years >= 0.0, "invalid inputs");
  return spot * std::pow((1.0 + domestic_rate) / (1.0 + foreign_rate), years);
}

double ForwardPremium(double spot, double forward) {
  INVESTM_CHECK(spot > 0.0, "spot must be positive");
  return forward / spot - 1.0;
}

double HedgedReturn(double foreign_return, double hedged_foreign_return,
                    double spot0, double forward, double spot1) {
  INVESTM_CHECK(spot0 > 0.0, "spot must be positive");
  // Sell forward the expected end value (1 + hedged_foreign_return); the
  // rest converts at the realized spot.
  const double hedged = (1.0 + hedged_foreign_return) * forward;
  const double unhedged = (foreign_return - hedged_foreign_return) * spot1;
  return (hedged + unhedged) / spot0 - 1.0;
}

double PppExpectedChange(double domestic_inflation, double foreign_inflation) {
  return (1.0 + domestic_inflation) / (1.0 + foreign_inflation) - 1.0;
}

double RealExchangeRate(double spot, double foreign_price_level,
                        double domestic_price_level) {
  INVESTM_CHECK(domestic_price_level > 0.0, "price level must be positive");
  return spot * foreign_price_level / domestic_price_level;
}

double TwoMarketStdDev(double foreign_weight, double domestic_std_dev,
                       double foreign_std_dev, double correlation) {
  const double wd = 1.0 - foreign_weight;
  const double wf = foreign_weight;
  return std::sqrt(wd * wd * domestic_std_dev * domestic_std_dev +
                   wf * wf * foreign_std_dev * foreign_std_dev +
                   2.0 * wd * wf * correlation * domestic_std_dev *
                       foreign_std_dev);
}

double MinimumRiskForeignWeight(double domestic_std_dev, double foreign_std_dev,
                                double correlation) {
  const double vd = domestic_std_dev * domestic_std_dev;
  const double vf = foreign_std_dev * foreign_std_dev;
  const double c = correlation * domestic_std_dev * foreign_std_dev;
  const double denom = vd + vf - 2.0 * c;
  INVESTM_CHECK(denom > 0.0,
                "markets are perfectly correlated with equal risk");
  return (vd - c) / denom;
}

}  // namespace investm::ch25
