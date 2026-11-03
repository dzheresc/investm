#include "cpp/ch19_options/payoffs.h"

#include <algorithm>
#include <cmath>

#include "cpp/core/check.h"

namespace investm::ch19 {

double Payoff(OptionType type, double strike, double spot_at_expiry) {
  return type == OptionType::kCall ? std::max(spot_at_expiry - strike, 0.0)
                                   : std::max(strike - spot_at_expiry, 0.0);
}

double StrategyPayoff(std::span<const Leg> legs, double spot_at_expiry) {
  double total = 0.0;
  for (const Leg& leg : legs) {
    double unit = 0.0;
    switch (leg.instrument) {
      case Instrument::kCall:
        unit = Payoff(OptionType::kCall, leg.strike, spot_at_expiry);
        break;
      case Instrument::kPut:
        unit = Payoff(OptionType::kPut, leg.strike, spot_at_expiry);
        break;
      case Instrument::kStock:
        unit = spot_at_expiry;
        break;
      case Instrument::kBond:
        unit = leg.strike;
        break;
    }
    total += leg.quantity * unit;
  }
  return total;
}

double StrategyProfit(std::span<const Leg> legs, double spot_at_expiry) {
  double cost = 0.0;
  for (const Leg& leg : legs) cost += leg.quantity * leg.premium;
  return StrategyPayoff(legs, spot_at_expiry) - cost;
}

double ParityPutPrice(double call, double spot, double strike, double rate,
                      double years) {
  INVESTM_CHECK(years >= 0.0, "time to expiry must be non-negative");
  return call - spot + strike * std::exp(-rate * years);
}

double ParityCallPrice(double put, double spot, double strike, double rate,
                       double years) {
  INVESTM_CHECK(years >= 0.0, "time to expiry must be non-negative");
  return put + spot - strike * std::exp(-rate * years);
}

double ParityDeviation(double call, double put, double spot, double strike,
                       double rate, double years) {
  return call - put - (spot - strike * std::exp(-rate * years));
}

double CallLowerBound(double spot, double strike, double rate, double years) {
  return std::max(spot - strike * std::exp(-rate * years), 0.0);
}

double PutLowerBound(double spot, double strike, double rate, double years) {
  return std::max(strike * std::exp(-rate * years) - spot, 0.0);
}

}  // namespace investm::ch19
