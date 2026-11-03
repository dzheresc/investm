// Chapter 23, Investment Management.
// Estimating an investor's risk tolerance, and the dynamic strategies used
// to manage an asset mix through time: buy-and-hold, constant mix
// (with or without a rebalancing band) and constant-proportion portfolio
// insurance (CPPI).

#ifndef CPP_CH23_INVESTMENT_MANAGEMENT_MANAGEMENT_H_
#define CPP_CH23_INVESTMENT_MANAGEMENT_MANAGEMENT_H_

#include <span>
#include <vector>

namespace investm::ch23 {

// 23.3 Risk tolerance implied by a chosen stock/bills mix, using the
// mean-variance utility U = E - sigma^2 / tau of Chapter 6: an investor
// who holds fraction w in stocks has tau = 2 sigma^2 w / (E - r_f).
double ImpliedRiskTolerance(double stock_weight, double stock_expected,
                            double stock_std_dev, double riskfree);
// Inverse: the optimal stock fraction for a given tolerance.
double OptimalStockWeight(double tau, double stock_expected,
                          double stock_std_dev, double riskfree);

// 23.4 Dynamic strategies over a path of risky-asset returns, with the
// rest in a riskless asset earning `riskfree` per period. Each returns the
// wealth path, starting with `initial_wealth` (size = returns + 1).
struct StrategyPath {
  std::vector<double> wealth;
  std::vector<double> risky_weight;  // At the start of each period.
  double turnover = 0.0;             // Sum of |trades| / wealth.
};

StrategyPath BuyAndHold(std::span<const double> risky_returns, double riskfree,
                        double initial_weight, double initial_wealth = 1.0);

// Rebalances to `target_weight` whenever the weight drifts more than
// `band` away (band = 0 rebalances every period).
StrategyPath ConstantMix(std::span<const double> risky_returns, double riskfree,
                         double target_weight, double band = 0.0,
                         double initial_wealth = 1.0);

// CPPI: risky exposure = multiplier x (wealth - floor), between 0 and
// max_weight x wealth; the floor starts at floor_fraction x wealth and
// grows at the riskless rate.
StrategyPath Cppi(std::span<const double> risky_returns, double riskfree,
                  double multiplier, double floor_fraction,
                  double max_weight = 1.0, double initial_wealth = 1.0);

}  // namespace investm::ch23

#endif  // CPP_CH23_INVESTMENT_MANAGEMENT_MANAGEMENT_H_
