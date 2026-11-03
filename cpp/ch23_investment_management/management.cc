#include "cpp/ch23_investment_management/management.h"

#include <algorithm>
#include <cmath>

#include "cpp/core/check.h"

namespace investm::ch23 {
namespace {

// Advances one period: grows the risky and riskless parts and returns the
// new wealth and the drifted risky weight.
void Grow(double wealth, double weight, double r, double rf, double* new_wealth,
          double* drifted_weight) {
  const double risky = wealth * weight * (1.0 + r);
  const double safe = wealth * (1.0 - weight) * (1.0 + rf);
  *new_wealth = risky + safe;
  *drifted_weight = *new_wealth != 0.0 ? risky / *new_wealth : 0.0;
}

// Sizes both series for the whole path up front, so a simulation does two
// allocations instead of one per doubling of each vector.
StrategyPath StartPath(std::size_t periods, double initial_wealth) {
  StrategyPath p;
  p.wealth.reserve(periods + 1);
  p.risky_weight.reserve(periods);
  p.wealth.push_back(initial_wealth);
  return p;
}

}  // namespace

double ImpliedRiskTolerance(double stock_weight, double stock_expected,
                            double stock_std_dev, double riskfree) {
  INVESTM_CHECK(stock_expected > riskfree, "need a positive risk premium");
  return 2.0 * stock_std_dev * stock_std_dev * stock_weight /
         (stock_expected - riskfree);
}

double OptimalStockWeight(double tau, double stock_expected,
                          double stock_std_dev, double riskfree) {
  INVESTM_CHECK(stock_std_dev > 0.0, "std dev must be positive");
  return tau * (stock_expected - riskfree) /
         (2.0 * stock_std_dev * stock_std_dev);
}

StrategyPath BuyAndHold(std::span<const double> risky_returns, double riskfree,
                        double initial_weight, double initial_wealth) {
  StrategyPath p = StartPath(risky_returns.size(), initial_wealth);
  double wealth = initial_wealth;
  double w = initial_weight;
  for (double r : risky_returns) {
    p.risky_weight.push_back(w);
    Grow(wealth, w, r, riskfree, &wealth, &w);
    p.wealth.push_back(wealth);
  }
  return p;
}

StrategyPath ConstantMix(std::span<const double> risky_returns, double riskfree,
                         double target_weight, double band,
                         double initial_wealth) {
  INVESTM_CHECK(band >= 0.0, "band must be non-negative");
  StrategyPath p = StartPath(risky_returns.size(), initial_wealth);
  double wealth = initial_wealth;
  double w = target_weight;
  bool first = true;
  for (double r : risky_returns) {
    if (!first && std::abs(w - target_weight) > band) {
      p.turnover += std::abs(w - target_weight);
      w = target_weight;
    }
    first = false;
    p.risky_weight.push_back(w);
    Grow(wealth, w, r, riskfree, &wealth, &w);
    p.wealth.push_back(wealth);
  }
  return p;
}

StrategyPath Cppi(std::span<const double> risky_returns, double riskfree,
                  double multiplier, double floor_fraction, double max_weight,
                  double initial_wealth) {
  INVESTM_CHECK(multiplier >= 0.0, "multiplier must be non-negative");
  INVESTM_CHECK(floor_fraction >= 0.0 && floor_fraction <= 1.0,
                "floor fraction in [0, 1]");
  StrategyPath p = StartPath(risky_returns.size(), initial_wealth);
  double wealth = initial_wealth;
  double floor = floor_fraction * initial_wealth;
  double drifted = 0.0;
  bool first = true;
  for (double r : risky_returns) {
    const double cushion = std::max(wealth - floor, 0.0);
    const double w = wealth > 0.0 ? std::clamp(multiplier * cushion / wealth,
                                               0.0, max_weight)
                                  : 0.0;
    if (!first) p.turnover += std::abs(w - drifted);
    first = false;
    p.risky_weight.push_back(w);
    Grow(wealth, w, r, riskfree, &wealth, &drifted);
    p.wealth.push_back(wealth);
    floor *= 1.0 + riskfree;
  }
  return p;
}

}  // namespace investm::ch23
