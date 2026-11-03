// Chapter 15, Bond Portfolio Management.
// Active management (horizon analysis, bond swaps, riding the yield curve,
// contingent immunization), passive management (immunization, cash
// matching) and interest rate swaps.

#ifndef CPP_CH15_BOND_PORTFOLIO_BOND_PORTFOLIO_H_
#define CPP_CH15_BOND_PORTFOLIO_BOND_PORTFOLIO_H_

#include <optional>
#include <span>
#include <vector>

#include "cpp/ch05_riskless_securities/cash_flows.h"

namespace investm::ch15 {

using ch05::CashFlows;

// 15.2.1 Horizon analysis of a coupon bond bought at `buy_yield` and sold
// after `horizon` years (a whole number of periods) at `end_yield`, with
// coupons reinvested at `reinvest_rate` (all quoted with `frequency`).
struct HorizonResult {
  double purchase_price;
  double sale_price;
  double coupon_income;
  double interest_on_interest;
  double total_return;             // (sale + coupons + interest) / buy - 1.
  double realized_compound_yield;  // Annualized, quoted like the yields.
};
HorizonResult HorizonAnalysis(double face, double coupon_rate,
                              double years_to_maturity, double buy_yield,
                              double horizon, double end_yield,
                              double reinvest_rate, int frequency = 2);

// 15.3 Value-weighted duration of a portfolio.
double PortfolioDuration(std::span<const double> values,
                         std::span<const double> durations);

// 15.3.2 Immunization with two bonds: values to hold in bonds with
// durations d1 < d2 so that the portfolio's value and duration match a
// liability's. nullopt if the liability duration is outside [d1, d2]
// (the solution would require a short position).
struct TwoBondHolding {
  double value1;
  double value2;
};
std::optional<TwoBondHolding> ImmunizeTwoBonds(double liability_pv,
                                               double liability_duration,
                                               double d1, double d2);

// Present value of assets minus liabilities at a flat yield.
double Surplus(const CashFlows& assets, const CashFlows& liabilities,
               double yield, int frequency = 1);

// Scales each bond's per-unit cash flows by `units` and merges them.
CashFlows Aggregate(const std::vector<CashFlows>& bonds,
                    std::span<const double> units);

// 15.3.3 Cash matching (dedication) by the classical backward pass: for
// each liability, from the last to the first, buy enough of the bond whose
// final cash flow falls on that date to cover what is still owed, then
// credit its earlier coupons to earlier liabilities. Requires one bond
// maturing on each liability date; returns nullopt otherwise. Units may
// be fractional; a liability already covered gets zero units.
std::optional<std::vector<double>> CashMatch(
    const std::vector<CashFlows>& bonds, const CashFlows& liabilities);

// 15.2.4 Contingent immunization. Assets needed now to lock in `floor`
// at the horizon at the current immunization rate, and the cushion above
// that. Active management may continue while the cushion is positive.
double RequiredImmunizedAssets(double floor, double rate, double horizon,
                               int frequency = 2);
double ContingentCushion(double assets, double floor, double rate,
                         double horizon, int frequency = 2);

// 15.4 Interest rate swaps on a schedule of discount factors D_1..D_n for
// equally spaced payment dates `period` years apart.
double ParSwapRate(std::span<const double> discount_factors, double period);
// Value to the fixed-rate payer: PV(floating) - PV(fixed), with the
// floating leg worth notional (1 - D_n) at a reset date.
double PayFixedSwapValue(double notional, double fixed_rate,
                         std::span<const double> discount_factors,
                         double period);

}  // namespace investm::ch15

#endif  // CPP_CH15_BOND_PORTFOLIO_BOND_PORTFOLIO_H_
