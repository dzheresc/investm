#include "cpp/ch20_futures/futures.h"

#include <cmath>
#include <vector>

#include "cpp/core/check.h"

namespace investm::ch20 {

std::vector<MarginDay> MarkToMarket(double entry_price,
                                    std::span<const double> settlements,
                                    double position, double multiplier,
                                    double initial_margin,
                                    double maintenance_margin) {
  INVESTM_CHECK(initial_margin >= maintenance_margin && maintenance_margin >= 0,
                "need initial >= maintenance >= 0");
  std::vector<MarginDay> days;
  days.reserve(settlements.size());
  double equity = initial_margin;
  double previous = entry_price;
  for (double price : settlements) {
    const double gain = position * multiplier * (price - previous);
    equity += gain;
    double call = 0.0;
    if (equity < maintenance_margin) {
      call = initial_margin - equity;
      equity = initial_margin;
    }
    days.push_back({price, gain, call, equity});
    previous = price;
  }
  return days;
}

double CostOfCarryPrice(double spot, double rate, double income_yield,
                        double storage_cost, double years) {
  INVESTM_CHECK(spot > 0.0 && years >= 0.0, "invalid inputs");
  return spot * std::exp((rate + storage_cost - income_yield) * years);
}

double IndexFuturesPrice(double spot, double rate,
                         std::span<const Dividend> dividends, double years) {
  double pv = 0.0;
  for (const Dividend& d : dividends) {
    if (d.time <= years) pv += d.amount * std::exp(-rate * d.time);
  }
  return (spot - pv) * std::exp(rate * years);
}

ArbitrageBand NoArbitrageBand(double spot, double lending_rate,
                              double borrowing_rate, double income_yield,
                              double cost, double years) {
  INVESTM_CHECK(borrowing_rate >= lending_rate, "borrowing >= lending");
  INVESTM_CHECK(cost >= 0.0, "cost must be non-negative");
  // Cash and carry (borrow, buy spot, sell futures) pays if F is above
  // upper; reverse cash and carry (short spot, lend, buy futures) pays if
  // F is below lower.
  return {
      spot * (1.0 - cost) * std::exp((lending_rate - income_yield) * years),
      spot * (1.0 + cost) * std::exp((borrowing_rate - income_yield) * years)};
}

stats::SimpleRegression MinimumVarianceHedge(
    std::span<const double> spot_changes,
    std::span<const double> futures_changes) {
  return stats::Ols(futures_changes, spot_changes);
}

double HedgeEffectiveness(std::span<const double> spot_changes,
                          std::span<const double> futures_changes, double h) {
  INVESTM_CHECK(spot_changes.size() == futures_changes.size(), "size mismatch");
  std::vector<double> hedged(spot_changes.size());
  for (std::size_t t = 0; t < hedged.size(); ++t) {
    hedged[t] = spot_changes[t] - h * futures_changes[t];
  }
  return 1.0 - stats::Variance(hedged) / stats::Variance(spot_changes);
}

double ContractsToHedge(double hedge_ratio, double exposure,
                        double contract_size) {
  INVESTM_CHECK(contract_size > 0.0, "contract size must be positive");
  return hedge_ratio * exposure / contract_size;
}

double IndexFuturesContracts(double portfolio_value, double current_beta,
                             double target_beta, double futures_price,
                             double multiplier) {
  INVESTM_CHECK(futures_price > 0.0 && multiplier > 0.0, "invalid contract");
  return (target_beta - current_beta) * portfolio_value /
         (futures_price * multiplier);
}

double BlackFuturesOptionPrice(ch19::OptionType type, double futures,
                               double strike, double rate, double volatility,
                               double years) {
  INVESTM_CHECK(futures > 0.0 && strike > 0.0, "prices must be positive");
  INVESTM_CHECK(volatility > 0.0 && years > 0.0, "invalid vol or time");
  const double sd = volatility * std::sqrt(years);
  const double d1 = (std::log(futures / strike) + 0.5 * sd * sd) / sd;
  const double d2 = d1 - sd;
  const double disc = std::exp(-rate * years);
  if (type == ch19::OptionType::kCall) {
    return disc * (futures * stats::NormCdf(d1) - strike * stats::NormCdf(d2));
  }
  return disc * (strike * stats::NormCdf(-d2) - futures * stats::NormCdf(-d1));
}

}  // namespace investm::ch20
