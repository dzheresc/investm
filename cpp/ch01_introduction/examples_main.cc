// Worked examples for Chapter 1. All numbers are illustrative.

#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "cpp/ch01_introduction/returns.h"
#include "cpp/core/stats.h"
#include "cpp/core/table.h"

namespace {

using investm::PrintHeading;
using investm::PrintPercent;

void ReturnOnOneHolding() {
  PrintHeading("1.1.2 Return on a single holding");
  // Bought at 40, received a 1.20 dividend, sold at 43.
  PrintPercent("HPR (40 -> 43, dividend 1.20)",
               investm::ch01::HoldingPeriodReturn(40.0, 43.0, 1.20));
}

void RiskAndReturnFromPrices() {
  PrintHeading("1.1.2 Risk and return from a price history");
  const std::vector<double> prices{100, 104, 101, 108, 112, 109, 115};
  const std::vector<double> r = investm::ch01::SimpleReturns(prices);
  PrintPercent("mean periodic return", investm::stats::Mean(r));
  PrintPercent("standard deviation", investm::stats::StdDev(r));
  PrintPercent("cumulative return", investm::ch01::CumulativeReturn(r));
  PrintPercent("geometric mean return", investm::stats::GeometricMeanReturn(r));
}

void Diversification() {
  PrintHeading("1.1.2 Diversification: equal-weight portfolio of N stocks");
  std::printf("  each stock: sigma = 40%%, pairwise correlation = 0.25\n");
  investm::Table table({"N", "sigma_p %"});
  for (int n : {1, 2, 5, 10, 20, 50, 100, 1000}) {
    table.AddRow(
        {static_cast<double>(n),
         100.0 * investm::ch01::EqualWeightPortfolioStdDev(n, 0.40, 0.25)},
        2);
  }
  table.Print();
  std::printf("  floor as N grows: sqrt(rho) * sigma = %.2f%%\n",
              100.0 * std::sqrt(0.25) * 0.40);
}

}  // namespace

int main() {
  ReturnOnOneHolding();
  RiskAndReturnFromPrices();
  Diversification();
  return 0;
}
