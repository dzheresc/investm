// Worked examples for Chapter 24. All numbers are illustrative; the
// return histories are simulated with a fixed seed.

#include <cstdio>
#include <random>
#include <vector>

#include "cpp/ch24_performance/performance.h"
#include "cpp/core/table.h"

namespace {

using investm::Matrix;
using investm::PrintHeading;
using investm::PrintPercent;
namespace ch24 = investm::ch24;

void Returns() {
  PrintHeading("24.2 Time-weighted vs dollar-weighted returns");
  // Quarterly values just before each flow, and the flows (deposits +).
  const std::vector<double> value_end{1060.0, 2010.0, 2150.0, 1890.0};
  const std::vector<double> flows{1000.0, 0.0, -500.0, 0.0};
  const auto r = ch24::PeriodReturnsFromValues(1000.0, value_end, flows);
  for (std::size_t t = 0; t < r.size(); ++t) {
    std::printf("  quarter %zu: %+6.2f%%  flow %+7.0f\n", t + 1, 100 * r[t],
                flows[t]);
  }
  PrintPercent("time-weighted (manager's skill)", ch24::TimeWeightedReturn(r));
  const std::vector<double> interim(flows.begin(), flows.end() - 1);
  const double dw =
      ch24::DollarWeightedReturn(1000.0, interim, value_end.back()).value();
  PrintPercent("dollar-weighted, per quarter (investor's timing)", dw);
}

void RiskAdjusted() {
  PrintHeading("24.4 Risk-adjusted performance of three managers (60 mo)");
  std::mt19937 rng(7);
  std::normal_distribution<double> mkt(0.007, 0.045);
  std::normal_distribution<double> eps(0.0, 0.015);
  const std::size_t n = 60;
  std::vector<double> m(n);
  std::vector<double> rf(n, 0.003);
  std::vector<double> a(n);
  std::vector<double> b(n);
  std::vector<double> c(n);
  for (std::size_t t = 0; t < n; ++t) {
    m[t] = mkt(rng);
    const double em = m[t] - rf[t];
    a[t] = rf[t] + 0.002 + 0.7 * em + eps(rng);      // Defensive stock-picker.
    b[t] = rf[t] + 0.000 + 1.4 * em + eps(rng);      // Levered index.
    c[t] = rf[t] - 0.001 + 1.0 * em + 2 * eps(rng);  // Expensive closet.
  }
  std::printf("  %-10s %8s %7s %8s %7s %8s %8s %8s %8s\n", "manager", "mean ex",
              "beta", "alpha", "t", "Sharpe", "Treynor", "M^2", "apprais");
  const char* names[] = {"A", "B", "C", "market"};
  const std::vector<double>* series[] = {&a, &b, &c, &m};
  for (int i = 0; i < 4; ++i) {
    const auto s = ch24::Evaluate(*series[i], m, rf);
    std::printf(
        "  %-10s %7.3f%% %7.3f %7.3f%% %7.2f %8.4f %7.3f%% %7.3f%% "
        "%8.4f\n",
        names[i], 100 * s.mean_excess, s.beta, 100 * s.alpha, s.alpha_t_stat,
        s.sharpe, 100 * s.treynor, 100 * s.m_squared, s.appraisal_ratio);
  }
  std::printf("  information ratio of A vs market: %.3f\n",
              ch24::InformationRatio(a, m));

  PrintHeading("24.6 Market timing tests");
  std::vector<double> timer(n);
  for (std::size_t t = 0; t < n; ++t) {
    const double em = m[t] - rf[t];
    timer[t] = rf[t] + (em > 0 ? 1.3 : 0.5) * em + 0.5 * eps(rng);
  }
  const auto tm = ch24::TreynorMazuy(timer, m, rf).value();
  const auto hm = ch24::HenrikssonMerton(timer, m, rf).value();
  std::printf("  Treynor-Mazuy:     beta %.3f, timing %.3f (t %.2f)\n", tm.beta,
              tm.timing, tm.timing_t_stat);
  std::printf("  Henriksson-Merton: beta %.3f, timing %.3f (t %.2f)\n", hm.beta,
              hm.timing, hm.timing_t_stat);
}

void Attribution() {
  PrintHeading("24.7 Brinson attribution (stocks, bonds, cash)");
  const std::vector<double> wp{0.68, 0.25, 0.07};
  const std::vector<double> wb{0.60, 0.35, 0.05};
  const std::vector<double> rp{0.052, 0.019, 0.011};
  const std::vector<double> rb{0.047, 0.021, 0.011};
  const auto a = ch24::BrinsonAttribution(wp, wb, rp, rb);
  const char* names[] = {"stocks", "bonds", "cash"};
  std::printf("  %-8s %11s %11s %11s\n", "segment", "allocation", "selection",
              "interaction");
  for (std::size_t i = 0; i < 3; ++i) {
    std::printf("  %-8s %10.3f%% %10.3f%% %10.3f%%\n", names[i],
                100 * a.allocation[i], 100 * a.selection[i],
                100 * a.interaction[i]);
  }
  std::printf("  portfolio %.3f%%, benchmark %.3f%%, active %.3f%%\n",
              100 * a.portfolio_return, 100 * a.benchmark_return,
              100 * a.Total());
}

void Style() {
  PrintHeading("24.8 Returns-based style analysis");
  std::mt19937 rng(240);
  std::normal_distribution<double> mkt(0.007, 0.045);
  std::normal_distribution<double> noise(0.0, 0.01);
  const std::size_t n = 60;
  Matrix styles(n, 4);
  std::vector<double> fund(n);
  for (std::size_t t = 0; t < n; ++t) {
    const double m = mkt(rng);
    styles(t, 0) = 0.003;
    styles(t, 1) = 0.002 + 0.15 * m + 0.8 * noise(rng);
    styles(t, 2) = 0.9 * m + noise(rng);
    styles(t, 3) = 1.2 * m + 1.5 * noise(rng);
    fund[t] = 0.1 * styles(t, 0) + 0.2 * styles(t, 1) + 0.25 * styles(t, 2) +
              0.45 * styles(t, 3) + 0.3 * noise(rng);
  }
  const auto fit = ch24::StyleAnalysis(fund, styles);
  const char* names[] = {"bills", "bonds", "value", "growth"};
  for (std::size_t i = 0; i < 4; ++i) {
    std::printf("  %-7s %6.3f\n", names[i], fit.weights[i]);
  }
  std::printf("  R^2 %.4f (true mix 0.10/0.20/0.25/0.45)\n", fit.r_squared);
}

}  // namespace

int main() {
  Returns();
  RiskAdjusted();
  Attribution();
  Style();
  return 0;
}
