#include "investm/formulas/derivatives.h"

#include <algorithm>
#include <cmath>
#include <numbers>

namespace investm::formulas {

double CallIntrinsicValue(double stock_price, double strike) {
  return std::max(0.0, stock_price - strike);
}

double PutIntrinsicValue(double stock_price, double strike) {
  return std::max(0.0, strike - stock_price);
}

double PutFromParity(double call_price, double stock_price, double strike,
                     double rate, double years) {
  return call_price + strike * std::exp(-rate * years) - stock_price;
}

double CallFromParity(double put_price, double stock_price, double strike,
                      double rate, double years) {
  return put_price + stock_price - strike * std::exp(-rate * years);
}

double NormalCdf(double x) { return 0.5 * std::erfc(-x / std::numbers::sqrt2); }

double BlackScholesD1(double stock_price, double strike, double rate,
                      double years, double volatility) {
  return (std::log(stock_price / strike) +
          (rate + 0.5 * volatility * volatility) * years) /
         (volatility * std::sqrt(years));
}

double BlackScholesD2(double stock_price, double strike, double rate,
                      double years, double volatility) {
  return BlackScholesD1(stock_price, strike, rate, years, volatility) -
         volatility * std::sqrt(years);
}

double BlackScholesCall(double stock_price, double strike, double rate,
                        double years, double volatility) {
  const double d1 =
      BlackScholesD1(stock_price, strike, rate, years, volatility);
  const double d2 = d1 - volatility * std::sqrt(years);
  return NormalCdf(d1) * stock_price -
         strike * std::exp(-rate * years) * NormalCdf(d2);
}

double BlackScholesPut(double stock_price, double strike, double rate,
                       double years, double volatility) {
  const double d1 =
      BlackScholesD1(stock_price, strike, rate, years, volatility);
  const double d2 = d1 - volatility * std::sqrt(years);
  return strike * std::exp(-rate * years) * NormalCdf(-d2) -
         stock_price * NormalCdf(-d1);
}

double FuturesFairValue(double spot_price, double interest, double benefits,
                        double costs) {
  return spot_price + interest - benefits + costs;
}

double IndexFuturesFairValue(double spot_price, double rate,
                             double dividend_yield) {
  return spot_price + rate * spot_price - dividend_yield * spot_price;
}

}  // namespace investm::formulas
