// Worked examples for Chapter 18. All numbers are illustrative; the
// earnings-surprise panel is simulated with a fixed seed.

#include <cstdio>
#include <random>
#include <vector>

#include "cpp/ch18_earnings/earnings.h"
#include "cpp/core/table.h"

namespace {

using investm::PrintHeading;
using investm::PrintValue;
namespace ch18 = investm::ch18;

void Lintner() {
  PrintHeading("18.3 Dividends follow earnings slowly (Lintner)");
  const std::vector<double> eps{3.10, 3.45, 2.80, 3.90, 4.25,
                                3.70, 4.60, 5.05, 4.40, 5.30};
  const std::vector<double> dps{1.20, 1.27, 1.25, 1.36, 1.49,
                                1.53, 1.65, 1.82, 1.85, 1.98};
  const auto m = ch18::FitLintner(eps, dps).value();
  std::printf("  dD = %.4f + %.3f (%.3f E - D_prev),  R^2 %.3f\n", m.a,
              m.adjustment, m.target_payout, m.r_squared);
  std::printf("  %4s %7s %7s %10s\n", "year", "EPS", "DPS", "model DPS");
  for (std::size_t t = 1; t < eps.size(); ++t) {
    std::printf("  %4zu %7.2f %7.2f %10.3f\n", t + 1, eps[t], dps[t],
                m.NextDividend(dps[t - 1], eps[t]));
  }
}

void Forecasts() {
  PrintHeading("18.4 Naive earnings forecasts");
  const std::vector<double> annual{2.10, 2.35, 2.28, 2.61, 2.90};
  PrintValue("random walk with drift, next year EPS",
             ch18::RandomWalkForecast(annual));
  const std::vector<double> q{0.42, 0.61, 0.55, 0.78, 0.47, 0.66,
                              0.58, 0.85, 0.50, 0.71, 0.63, 0.90};
  const double f = ch18::SeasonalRandomWalkForecast(q);
  PrintValue("seasonal random walk, next quarter EPS", f);
  for (double actual : {0.49, 0.55, 0.60}) {
    std::printf("  actual %.2f -> SUE %+.2f\n", actual,
                ch18::StandardizedUnexpectedEarnings(q, actual));
  }
}

void Drift() {
  PrintHeading("18.5 Returns after earnings announcements, by SUE quintile");
  // Simulated panel: next-quarter abnormal return = 0.5% x SUE + noise.
  std::mt19937 rng(18);
  std::normal_distribution<double> sue_dist(0.0, 1.5);
  std::normal_distribution<double> noise(0.0, 0.03);
  const int n = 2000;
  std::vector<double> sue(n);
  std::vector<double> car(n);
  for (std::size_t i = 0; i < sue.size(); ++i) {
    sue[i] = sue_dist(rng);
    car[i] = 0.005 * sue[i] + noise(rng);
  }
  const auto group = ch18::SortIntoGroups(sue, 5);
  const auto mean_sue = ch18::GroupMeans(sue, group, 5);
  const auto mean_car = ch18::GroupMeans(car, group, 5);
  std::printf("  %9s %10s %16s\n", "quintile", "mean SUE", "mean abn. return");
  for (int k = 0; k < 5; ++k) {
    std::printf("  %9d %10.2f %15.2f%%\n", k + 1,
                mean_sue[static_cast<std::size_t>(k)],
                100 * mean_car[static_cast<std::size_t>(k)]);
  }
}

}  // namespace

int main() {
  Lintner();
  Forecasts();
  Drift();
  return 0;
}
