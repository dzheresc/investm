// Worked examples for Chapter 23. All numbers are illustrative; market
// paths are simulated with a fixed seed.

#include <algorithm>
#include <cstdio>
#include <initializer_list>
#include <random>
#include <vector>

#include "cpp/ch07_portfolio_analysis/efficient_set.h"
#include "cpp/ch23_investment_management/management.h"
#include "cpp/core/matrix.h"
#include "cpp/core/table.h"

namespace {

using investm::PrintHeading;
namespace ch07 = investm::ch07;
namespace ch23 = investm::ch23;

void RiskTolerance() {
  PrintHeading("23.3 Risk tolerance (stocks 8.5% +/- 17%, bills 3.5%)");
  for (double w : {0.2, 0.4, 0.6, 0.8, 1.0}) {
    std::printf("  holds %3.0f%% stocks -> tau = %.3f\n", 100 * w,
                ch23::ImpliedRiskTolerance(w, 0.085, 0.17, 0.035));
  }
}

void StrategicAllocation() {
  PrintHeading("23.4 Strategic asset allocation (no short sales)");
  const investm::Vector mu{0.035, 0.050, 0.075, 0.090};
  const investm::Matrix cov = investm::CovarianceFromCorrelation(
      investm::Vector{0.01, 0.06, 0.16, 0.21},
      investm::Matrix({{1.0, 0.1, 0.0, 0.0},
                       {0.1, 1.0, 0.2, 0.1},
                       {0.0, 0.2, 1.0, 0.7},
                       {0.0, 0.1, 0.7, 1.0}}));
  std::printf("  %6s %8s %8s %8s %8s %8s %8s\n", "tau", "bills", "bonds",
              "US eq", "intl eq", "E %", "sd %");
  for (double tau : {0.05, 0.15, 0.30, 0.60}) {
    const auto p = ch07::OptimalNoShortSales(mu, cov, tau);
    std::printf("  %6.2f %8.3f %8.3f %8.3f %8.3f %8.2f %8.2f\n", tau,
                p.weights[0], p.weights[1], p.weights[2], p.weights[3],
                100 * p.expected_return, 100 * p.std_dev);
  }
}

void DynamicStrategies() {
  PrintHeading("23.4 Dynamic strategies over 2,000 simulated 10-year paths");
  std::mt19937 rng(23);
  std::normal_distribution<double> monthly(0.007, 0.045);
  const double rf = 0.003;
  const int paths = 2000;
  const int months = 120;
  std::vector<double> hold;
  std::vector<double> mix;
  std::vector<double> cppi;
  double mix_turnover = 0.0;
  double cppi_turnover = 0.0;
  hold.reserve(paths);
  mix.reserve(paths);
  cppi.reserve(paths);
  std::vector<double> r(months);
  for (int k = 0; k < paths; ++k) {
    for (double& x : r) x = monthly(rng);
    hold.push_back(ch23::BuyAndHold(r, rf, 0.6).wealth.back());
    const auto m = ch23::ConstantMix(r, rf, 0.6, 0.05);
    mix.push_back(m.wealth.back());
    mix_turnover += m.turnover;
    const auto c = ch23::Cppi(r, rf, 3.0, 0.8);
    cppi.push_back(c.wealth.back());
    cppi_turnover += c.turnover;
  }
  // Sort each sample once; every percentile is then a lookup.
  for (auto* v : {&hold, &mix, &cppi}) std::sort(v->begin(), v->end());
  auto pct = [](const std::vector<double>& v, double q) {
    return v[static_cast<std::size_t>(q * static_cast<double>(v.size() - 1))];
  };
  std::printf("  terminal wealth per $1: %10s %8s %8s %8s %10s\n", "5th pct",
              "median", "95th pct", "min", "turnover");
  std::printf("  %-22s %10.3f %8.3f %8.3f %8.3f %10s\n", "buy & hold 60/40",
              pct(hold, 0.05), pct(hold, 0.5), pct(hold, 0.95), pct(hold, 0.0),
              "0");
  std::printf("  %-22s %10.3f %8.3f %8.3f %8.3f %10.2f\n", "constant mix 60/40",
              pct(mix, 0.05), pct(mix, 0.5), pct(mix, 0.95), pct(mix, 0.0),
              mix_turnover / paths);
  std::printf("  %-22s %10.3f %8.3f %8.3f %8.3f %10.2f\n",
              "CPPI m=3, floor 80%", pct(cppi, 0.05), pct(cppi, 0.5),
              pct(cppi, 0.95), pct(cppi, 0.0), cppi_turnover / paths);
}

}  // namespace

int main() {
  RiskTolerance();
  StrategicAllocation();
  DynamicStrategies();
  return 0;
}
