// Worked examples for Chapter 4. All data are simulated.

#include <cstdio>
#include <random>
#include <utility>
#include <vector>

#include "cpp/ch04_efficient_markets/efficiency_tests.h"
#include "cpp/ch04_efficient_markets/schedules.h"
#include "cpp/core/stats.h"
#include "cpp/core/table.h"

namespace {

using investm::PrintHeading;
using investm::PrintPercent;
using investm::PrintValue;
namespace ch04 = investm::ch04;

void Schedules() {
  PrintHeading("4.1 Demand-to-buy and supply-to-sell schedules");
  std::vector<ch04::Investor> investors{
      {48.0, 1000, 800}, {50.0, 600, 1000}, {52.0, 400, 900},
      {55.0, 200, 1200}, {45.0, 1500, 500}, {58.0, 0, 700},
  };
  investm::Table table({"price", "demand", "supply"});
  for (double p = 46.0; p <= 57.0; p += 1.0) {
    table.AddRow({p, ch04::DemandToBuy(investors, p, 5.0),
                  ch04::SupplyToSell(investors, p, 5.0)},
                 1);
  }
  table.Print();
  const double p = ch04::EquilibriumPrice(investors).value();
  PrintValue("equilibrium price", p, "$");
  PrintValue("volume at equilibrium", ch04::DemandToBuy(investors, p, 5.0),
             "sh");
  std::printf("  4.1.5 Good news raises every valuation by 5%%:\n");
  for (auto& i : investors) i.value *= 1.05;
  PrintValue("new equilibrium price", ch04::EquilibriumPrice(investors).value(),
             "$");
}

void WeakFormTests() {
  PrintHeading("4.2.3 / 4.3.2 Weak-form tests on a simulated random walk");
  const auto prices = ch04::RandomWalkPrices(100.0, 2000, 0.0003, 0.012, 42);
  const auto r = ch04::LogReturns(prices);
  PrintValue("lag-1 autocorrelation", investm::stats::Autocorrelation(r, 1));
  const auto runs = ch04::RunsTest(r);
  std::printf("  runs: %zu observed vs %.1f expected, z = %.3f\n", runs.runs,
              runs.expected_runs, runs.z);
  for (std::size_t q : {2u, 5u, 10u, 20u}) {
    std::printf("  variance ratio VR(%2zu) = %.4f\n", q,
                ch04::VarianceRatio(r, q));
  }
  PrintPercent("buy-and-hold return", prices.back() / prices.front() - 1.0);
  for (double x : {0.01, 0.05, 0.10}) {
    std::printf("  %4.0f%% filter rule return %24.4f %%\n", 100.0 * x,
                100.0 * ch04::FilterRuleReturn(prices, x));
  }
}

void EventStudy() {
  PrintHeading("4.3.1 Event study: 30 firms, good news on day 0");
  std::mt19937_64 rng(7);
  std::normal_distribution<double> market(0.0004, 0.01);
  std::normal_distribution<double> noise(0.0, 0.015);
  constexpr std::size_t kEstimation = 120;
  constexpr std::size_t kEvent = 11;  // Days -5 to +5.
  std::vector<ch04::EventFirm> firms;
  for (int f = 0; f < 30; ++f) {
    ch04::EventFirm firm;
    const double beta = 0.7 + 0.02 * f;
    for (std::size_t d = 0; d < kEstimation + kEvent; ++d) {
      const double m = market(rng);
      double r = 0.0001 + beta * m + noise(rng);
      if (d == kEstimation + 5) r += 0.03;  // Announcement day.
      firm.market.push_back(m);
      firm.stock.push_back(r);
    }
    firms.push_back(std::move(firm));
  }
  const auto result = ch04::EventStudy(firms, kEstimation);
  investm::Table table({"day", "AAR %", "CAAR %"});
  for (std::size_t i = 0; i < result.aar.size(); ++i) {
    table.AddRow({static_cast<double>(i) - 5.0, 100.0 * result.aar[i],
                  100.0 * result.caar[i]},
                 3);
  }
  table.Print();
  PrintValue("t statistic of final CAAR", result.caar_t_stat);
  std::printf(
      "  In an efficient market the jump happens on day 0 and the CAAR is\n"
      "  flat afterwards.\n");
}

}  // namespace

int main() {
  Schedules();
  WeakFormTests();
  EventStudy();
  return 0;
}
