// Worked examples for Chapter 20. All numbers are illustrative.

#include <cmath>
#include <cstdio>
#include <vector>

#include "cpp/ch20_futures/futures.h"
#include "cpp/core/table.h"

namespace {

using investm::PrintHeading;
using investm::PrintPercent;
using investm::PrintValue;
namespace ch19 = investm::ch19;
namespace ch20 = investm::ch20;

void Margin() {
  PrintHeading("20.2 Marking to market: short 3 contracts x 1,000 at 72.40");
  std::printf("  initial margin $9,000, maintenance $6,500\n");
  const std::vector<double> settle{72.10, 73.05, 74.20, 73.60, 75.10, 74.00};
  const auto days =
      ch20::MarkToMarket(72.40, settle, -3.0, 1000.0, 9000.0, 6500.0);
  std::printf("  %4s %9s %10s %12s %10s\n", "day", "settle", "gain",
              "margin call", "equity");
  for (std::size_t t = 0; t < days.size(); ++t) {
    std::printf("  %4zu %9.2f %10.2f %12.2f %10.2f\n", t + 1,
                days[t].settlement_price, days[t].gain, days[t].margin_call,
                days[t].equity);
  }
}

void Carry() {
  PrintHeading("20.5 Futures prices and the cost of carry");
  PrintValue("gold: S 1,950, r 4.5%, storage 0.3%, 9 months",
             ch20::CostOfCarryPrice(1950.0, 0.045, 0.0, 0.003, 0.75));
  PrintValue("index: S 4,800, r 4.5%, yield 1.6%, 6 months",
             ch20::CostOfCarryPrice(4800.0, 0.045, 0.016, 0.0, 0.5));
  const std::vector<ch20::Dividend> divs{{1.0 / 12, 6.0}, {4.0 / 12, 8.0}};
  PrintValue("index with two discrete dividends, 5 months",
             ch20::IndexFuturesPrice(950.0, 0.045, divs, 5.0 / 12));
  const auto band =
      ch20::NoArbitrageBand(4800.0, 0.042, 0.050, 0.016, 0.001, 0.5);
  std::printf(
      "  arbitrage band (lend 4.2%%, borrow 5.0%%, 10bp cost): "
      "[%.2f, %.2f]\n",
      band.lower, band.upper);
  std::printf(
      "  convergence of the basis as expiry nears (r 4.5%%, q 1.6%%):"
      "\n");
  for (double months : {6.0, 4.0, 2.0, 1.0, 0.0}) {
    const double f =
        ch20::CostOfCarryPrice(4800.0, 0.045, 0.016, 0.0, months / 12);
    std::printf("    %3.0f months: F = %8.2f, basis = %7.2f\n", months, f,
                ch20::Basis(4800.0, f));
  }
}

void Hedging() {
  PrintHeading("20.4 A minimum-variance cross hedge");
  // Weekly changes: jet fuel (spot) hedged with heating oil futures.
  const std::vector<double> df{0.021, -0.034, 0.012,  0.045, -0.018, -0.027,
                               0.009, 0.038,  -0.041, 0.016, -0.006, 0.029};
  const std::vector<double> ds{0.017, -0.031, 0.015,  0.036, -0.019, -0.020,
                               0.004, 0.035,  -0.033, 0.010, -0.002, 0.024};
  const auto fit = ch20::MinimumVarianceHedge(ds, df);
  PrintValue("hedge ratio", fit.beta);
  PrintPercent("variance removed at h*",
               ch20::HedgeEffectiveness(ds, df, fit.beta));
  PrintPercent("variance removed at h = 1",
               ch20::HedgeEffectiveness(ds, df, 1.0));
  PrintValue("contracts for 2.5m gallons (42,000/contract)",
             ch20::ContractsToHedge(fit.beta, 2.5e6, 42000.0));

  PrintHeading("20.4 Changing a portfolio's beta with index futures");
  std::printf("  $35m portfolio, beta 1.15, futures 4,850 x $50\n");
  for (double target : {0.0, 0.5, 1.0, 1.5}) {
    std::printf("  target beta %.1f: %+8.1f contracts\n", target,
                ch20::IndexFuturesContracts(3.5e7, 1.15, target, 4850.0, 50.0));
  }
}

void FuturesOptions() {
  PrintHeading("20.6 Options on futures (Black's model)");
  std::printf("  F 82.50, r 4.5%%, sigma 32%%, 3 months\n");
  for (double k : {75.0, 80.0, 85.0, 90.0}) {
    std::printf("  K %5.1f: call %6.3f  put %6.3f\n", k,
                ch20::BlackFuturesOptionPrice(ch19::OptionType::kCall, 82.5, k,
                                              0.045, 0.32, 0.25),
                ch20::BlackFuturesOptionPrice(ch19::OptionType::kPut, 82.5, k,
                                              0.045, 0.32, 0.25));
  }
}

}  // namespace

int main() {
  Margin();
  Carry();
  Hedging();
  FuturesOptions();
  return 0;
}
