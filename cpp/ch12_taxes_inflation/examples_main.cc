// Worked examples for Chapter 12. All numbers are illustrative.

#include <cstdio>
#include <utility>
#include <vector>

#include "cpp/ch12_taxes_inflation/inflation.h"
#include "cpp/ch12_taxes_inflation/taxes.h"
#include "cpp/core/table.h"

namespace {

using investm::PrintHeading;
using investm::PrintPercent;
using investm::PrintValue;
namespace ch12 = investm::ch12;

void Schedules() {
  PrintHeading("12.1 A progressive schedule: marginal vs average rates");
  const ch12::TaxSchedule s(
      {{0.0, 0.10}, {25000.0, 0.18}, {70000.0, 0.27}, {160000.0, 0.36}});
  std::printf("  %10s %10s %10s %10s\n", "income", "tax", "marginal",
              "average");
  for (double y : {15000.0, 45000.0, 90000.0, 250000.0}) {
    std::printf("  %10.0f %10.0f %9.1f%% %9.2f%%\n", y, s.Tax(y),
                100 * s.MarginalRate(y), 100 * s.AverageRate(y));
  }
}

void TaxExempt() {
  PrintHeading("12.1.3 Tax-exempt bonds: taxable-equivalent yield");
  const double muni = 0.032;
  for (double fed : {0.12, 0.24, 0.32, 0.37}) {
    const double t = ch12::CombinedTaxRate(fed, 0.06, true);
    std::printf(
        "  federal %4.0f%% + state 6%% -> combined %5.2f%%, "
        "taxable-equivalent of 3.20%% muni = %5.2f%%\n",
        100 * fed, 100 * t, 100 * ch12::TaxableEquivalentYield(muni, t));
  }
  PrintPercent("break-even rate, 5.00% taxable vs 3.20% muni",
               ch12::BreakevenTaxRate(0.05, muni));
}

void Deferral() {
  PrintHeading("12.1.2 The value of deferring tax (8% pretax, 30% tax)");
  std::printf("  %6s %12s %12s %12s %10s\n", "years", "annual tax",
              "tax at end", "no tax", "eff. rate");
  for (double n : {1.0, 5.0, 10.0, 20.0, 40.0}) {
    std::printf("  %6.0f %12.2f %12.2f %12.2f %9.3f%%\n", n,
                ch12::WealthTaxedAnnually(1000.0, 0.08, 0.30, n),
                ch12::WealthTaxedAtEnd(1000.0, 0.08, 0.30, n),
                ch12::WealthUntaxed(1000.0, 0.08, n),
                100 * ch12::EffectiveDeferredRate(0.08, 0.30, n));
  }
}

void Inflation() {
  PrintHeading("12.2 Inflation and real returns");
  const std::vector<double> index{180.0, 185.4, 192.8, 194.7, 201.5, 213.6};
  const std::vector<double> nominal{0.12, -0.04, 0.09, 0.15, 0.03};
  const auto infl = ch12::InflationRates(index);
  const auto real = ch12::RealReturns(nominal, infl);
  std::printf("  %4s %10s %10s %10s\n", "year", "nominal", "inflation", "real");
  for (std::size_t t = 0; t < real.size(); ++t) {
    std::printf("  %4zu %9.2f%% %9.2f%% %9.2f%%\n", t + 1, 100 * nominal[t],
                100 * infl[t], 100 * real[t]);
  }
  PrintPercent("cumulative inflation", ch12::CumulativeInflation(index));
  PrintValue("purchasing power of $1", ch12::PurchasingPower(index));
  const auto beta = ch12::InflationBeta(nominal, infl);
  std::printf("  inflation beta of this asset: %.3f (R^2 %.3f)\n", beta.beta,
              beta.r_squared);

  PrintHeading("12.2.4 Taxes on nominal returns (30% tax)");
  for (auto [r, i] :
       {std::pair{0.05, 0.01}, std::pair{0.07, 0.03}, std::pair{0.10, 0.06}}) {
    std::printf(
        "  nominal %4.1f%%, inflation %4.1f%%: real after tax "
        "%5.2f%%, effective tax on real return %5.1f%%\n",
        100 * r, 100 * i, 100 * ch12::RealAfterTaxReturn(r, 0.30, i),
        100 * ch12::EffectiveTaxOnRealReturn(r, 0.30, i));
  }
}

void IndexedBond() {
  PrintHeading("12.2.5 An inflation-indexed bond");
  const std::vector<double> path{102.5, 105.1, 107.2, 110.4};
  const auto flows = ch12::IndexedBondCashFlows(1000.0, 0.015, 100.0, path);
  for (const auto& f : flows) {
    std::printf("  t=%.0f  pays %9.2f\n", f.time, f.amount);
  }
  const double be = ch12::BreakevenInflation(0.045, 0.015);
  PrintPercent("break-even inflation, 4.5% nominal vs 1.5% real", be);
}

}  // namespace

int main() {
  Schedules();
  TaxExempt();
  Deferral();
  Inflation();
  IndexedBond();
  return 0;
}
