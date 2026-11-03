// Chapter 4, Efficient Markets, Investment Value, and Market Price.
// Section 4.1: demand-to-buy and supply-to-sell schedules.
//
// Each investor has an estimate of a stock's investment value and holds some
// shares. At market price P, investors who value the stock above P want to
// buy and holders who value it below P want to sell. The market price is
// where the two schedules meet.

#ifndef CPP_CH04_EFFICIENT_MARKETS_SCHEDULES_H_
#define CPP_CH04_EFFICIENT_MARKETS_SCHEDULES_H_

#include <optional>
#include <span>

namespace investm::ch04 {

struct Investor {
  double value;        // The investor's estimate of investment value.
  double shares_held;  // Current holdings.
  double max_buy;      // Most shares the investor would add at a bargain.
};

// Quantities scale linearly with the percentage mispricing (times
// `sensitivity`) and are capped by max_buy or shares_held, so both schedules
// are continuous and monotone.
double DemandToBuy(std::span<const Investor> investors, double price,
                   double sensitivity);
double SupplyToSell(std::span<const Investor> investors, double price,
                    double sensitivity);

// 4.1.3: the price at which excess demand is zero. Returns nullopt when all
// investors agree on the value (no trade, no unique price).
std::optional<double> EquilibriumPrice(std::span<const Investor> investors,
                                       double sensitivity = 5.0);

// 4.1.4: arc (midpoint) elasticity between two points of a schedule.
double ArcElasticity(double p1, double q1, double p2, double q2);

}  // namespace investm::ch04

#endif  // CPP_CH04_EFFICIENT_MARKETS_SCHEDULES_H_
