// Worked examples for Chapter 15. All numbers are illustrative.

#include <cmath>
#include <cstdio>
#include <vector>

#include "cpp/ch05_riskless_securities/cash_flows.h"
#include "cpp/ch14_bond_analysis/duration.h"
#include "cpp/ch15_bond_portfolio/bond_portfolio.h"
#include "cpp/core/table.h"

namespace {

using investm::PrintHeading;
using investm::PrintPercent;
using investm::PrintValue;
namespace ch05 = investm::ch05;
namespace ch14 = investm::ch14;
namespace ch15 = investm::ch15;

void Horizon() {
  PrintHeading("15.2.1 Horizon analysis: 5.25% 10-year bought at 5.60%");
  std::printf("  3-year horizon, coupons reinvested at 4.5%%\n");
  std::printf("  %9s %9s %9s %9s %10s %9s\n", "end yld", "sale", "coupons",
              "int/int", "total ret", "RCY");
  for (double end : {0.045, 0.052, 0.056, 0.060, 0.067}) {
    const auto r =
        ch15::HorizonAnalysis(100.0, 0.0525, 10.0, 0.056, 3.0, end, 0.045);
    std::printf("  %8.2f%% %9.3f %9.3f %9.3f %9.2f%% %8.3f%%\n", 100 * end,
                r.sale_price, r.coupon_income, r.interest_on_interest,
                100 * r.total_return, 100 * r.realized_compound_yield);
  }
}

void Swap() {
  PrintHeading("15.2.2 A substitution swap with a one-year workout");
  // Two near-identical 8-year bonds; B yields 25bp more. If the spread
  // closes within a year, B gains on top of its higher yield.
  const auto a =
      ch15::HorizonAnalysis(100.0, 0.05, 8.0, 0.055, 1.0, 0.055, 0.05);
  const auto b =
      ch15::HorizonAnalysis(100.0, 0.05, 8.0, 0.0575, 1.0, 0.055, 0.05);
  PrintPercent("hold A (5.50%), 1-year RCY", a.realized_compound_yield);
  PrintPercent("swap into B (5.75%), 1-year RCY", b.realized_compound_yield);
  PrintValue("pickup (bp)",
             1e4 * (b.realized_compound_yield - a.realized_compound_yield));
}

void Riding() {
  PrintHeading("15.2.3 Riding an upward-sloping yield curve");
  const std::vector<double> spot{0.040, 0.046, 0.050, 0.053};
  for (std::size_t n = 1; n <= spot.size(); ++n) {
    const double years = static_cast<double>(n);
    const double buy = std::pow(1.0 + spot[n - 1], -years);
    const double sell =
        n == 1 ? 1.0 : std::pow(1.0 + spot[n - 2], -(years - 1.0));
    std::printf(
        "  buy %zu-year zero, sell after 1 year on an unchanged "
        "curve: %5.2f%%\n",
        n, 100 * (sell / buy - 1.0));
  }
}

void Immunize() {
  PrintHeading("15.3.2 Immunizing a 6-year, $5 million liability at 5.4%");
  const double y = 0.054;
  const ch15::CashFlows liability{{6.0, 5e6}};
  const double pv = ch05::PresentValueAtYield(liability, y);
  const auto b1 = ch05::CouponBond(100.0, 0.04, 3.0);
  const auto b2 = ch05::CouponBond(100.0, 0.06, 15.0);
  const double d1 = ch14::MacaulayDuration(b1, y);
  const double d2 = ch14::MacaulayDuration(b2, y);
  const auto h = ch15::ImmunizeTwoBonds(pv, 6.0, d1, d2).value();
  const std::vector<double> units{h.value1 / ch05::PresentValueAtYield(b1, y),
                                  h.value2 / ch05::PresentValueAtYield(b2, y)};
  std::printf("  bond durations %.3f and %.3f; hold $%.0f and $%.0f\n", d1, d2,
              h.value1, h.value2);
  const auto assets = ch15::Aggregate({b1, b2}, units);
  std::printf("  %8s %16s\n", "yield", "surplus ($)");
  for (double dy : {-0.02, -0.01, 0.0, 0.01, 0.02}) {
    std::printf("  %7.2f%% %16.2f\n", 100 * (y + dy),
                ch15::Surplus(assets, liability, y + dy));
  }
}

void Dedicate() {
  PrintHeading("15.3.3 Cash matching a pension payout schedule");
  const std::vector<ch15::CashFlows> bonds{
      ch05::CouponBond(1000.0, 0.035, 1.0),
      ch05::CouponBond(1000.0, 0.040, 2.0),
      ch05::CouponBond(1000.0, 0.045, 3.0),
      ch05::CouponBond(1000.0, 0.050, 4.0)};
  const std::vector<double> prices{998.0, 1001.5, 1003.0, 1008.0};
  const ch15::CashFlows payouts{
      {1.0, 2.0e6}, {2.0, 2.2e6}, {3.0, 2.4e6}, {4.0, 2.6e6}};
  const auto units = ch15::CashMatch(bonds, payouts).value();
  double cost = 0.0;
  for (std::size_t i = 0; i < units.size(); ++i) {
    std::printf("  bond maturing year %zu: %9.2f bonds\n", i + 1, units[i]);
    cost += units[i] * prices[i];
  }
  PrintValue("cost of the dedicated portfolio ($)", cost);
}

void Contingent() {
  PrintHeading("15.2.4 Contingent immunization: $100m, floor $125m in 5y");
  std::printf("  %10s %14s %12s\n", "rate", "required ($m)", "cushion ($m)");
  for (double r : {0.050, 0.060, 0.070}) {
    std::printf("  %9.2f%% %14.2f %12.2f\n", 100 * r,
                ch15::RequiredImmunizedAssets(125.0, r, 5.0),
                ch15::ContingentCushion(100.0, 125.0, r, 5.0));
  }
}

void InterestRateSwaps() {
  PrintHeading("15.4 Interest rate swaps (semiannual, 3 years)");
  std::vector<double> df;
  for (int k = 1; k <= 6; ++k) {
    df.push_back(std::pow(1.0 + (0.042 + 0.002 * k) / 2.0, -k));
  }
  const double par = ch15::ParSwapRate(df, 0.5);
  PrintPercent("par swap rate", par);
  for (double fixed : {par - 0.005, par, par + 0.005}) {
    std::printf("  pay %5.3f%% fixed on $50m: value %12.2f\n", 100 * fixed,
                ch15::PayFixedSwapValue(5e7, fixed, df, 0.5));
  }
}

}  // namespace

int main() {
  Horizon();
  Swap();
  Riding();
  Immunize();
  Dedicate();
  Contingent();
  InterestRateSwaps();
  return 0;
}
