// Worked examples for Chapter 22. All numbers are illustrative; the price
// series for the technical rules is simulated with a fixed seed.

#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

#include "cpp/ch22_financial_analysis/ratios.h"
#include "cpp/ch22_financial_analysis/technical.h"
#include "cpp/core/stats.h"
#include "cpp/core/table.h"

namespace {

using investm::PrintHeading;
using investm::PrintPercent;
namespace ch22 = investm::ch22;

ch22::Financials Firm(double debt, double interest) {
  ch22::Financials f;
  f.revenue = 2400.0;
  f.ebit = 312.0;
  f.interest_expense = interest;
  f.income_tax = 0.25 * (f.ebit - interest);
  f.total_assets = 1950.0;
  f.equity = 1950.0 - debt - 150.0;
  f.current_assets = 640.0;
  f.inventory = 210.0;
  f.current_liabilities = 380.0;
  f.total_debt = debt;
  f.dividends = 70.0;
  f.shares = 120.0;
  f.price = 18.40;
  return f;
}

void RatioAnalysis() {
  PrintHeading("22.3 Ratio analysis and the DuPont decomposition");
  std::printf("  same business, two capital structures (25%% tax)\n");
  std::printf("  %-22s %12s %12s\n", "", "low debt", "high debt");
  const ch22::Financials lo = Firm(300.0, 18.0);
  const ch22::Financials hi = Firm(1000.0, 64.0);
  const auto a = ch22::ComputeRatios(lo);
  const auto b = ch22::ComputeRatios(hi);
  const auto da = ch22::DecomposeRoe(lo);
  const auto db = ch22::DecomposeRoe(hi);
  auto row = [](const char* name, double x, double y) {
    std::printf("  %-22s %12.4f %12.4f\n", name, x, y);
  };
  row("tax burden", da.tax_burden, db.tax_burden);
  row("interest burden", da.interest_burden, db.interest_burden);
  row("operating margin", da.operating_margin, db.operating_margin);
  row("asset turnover", da.asset_turnover, db.asset_turnover);
  row("equity multiplier", da.equity_multiplier, db.equity_multiplier);
  row("ROE", a.roe, b.roe);
  row("ROA", a.roa, b.roa);
  row("debt / equity", a.debt_to_equity, b.debt_to_equity);
  row("interest coverage", a.interest_coverage, b.interest_coverage);
  row("current ratio", a.current_ratio, b.current_ratio);
  row("quick ratio", a.quick_ratio, b.quick_ratio);
  row("EPS", a.eps, b.eps);
  row("P/E at $18.40", a.price_earnings, b.price_earnings);
  row("price / book", a.price_to_book, b.price_to_book);
}

void Technical() {
  PrintHeading("22.4 Technical analysis on a simulated random walk");
  std::mt19937 rng(22);
  std::normal_distribution<double> step(0.0003, 0.012);
  std::vector<double> p{100.0};
  for (int t = 0; t < 1000; ++t) p.push_back(p.back() * std::exp(step(rng)));
  const auto ma = ch22::SimpleMovingAverage(p, 50);
  const auto mom = ch22::Momentum(p, 20);
  std::printf("  last price %.2f, 50-day MA %.2f, 20-day momentum %+.2f%%\n",
              p.back(), ma.back(), 100 * mom.back());
  const double cash = 0.0001;
  std::vector<double> hold(p.size() - 1);
  for (std::size_t t = 0; t + 1 < p.size(); ++t) {
    hold[t] = p[t + 1] / p[t] - 1.0;
  }
  std::printf("  %-18s %12s %12s\n", "strategy", "mean/day", "sd/day");
  std::printf("  %-18s %11.4f%% %11.4f%%\n", "buy and hold",
              100 * investm::stats::Mean(hold),
              100 * investm::stats::StdDev(hold));
  for (std::size_t n : {std::size_t{20}, std::size_t{50}, std::size_t{200}}) {
    const auto r = ch22::MovingAverageRuleReturns(p, n, cash);
    std::printf("  %3zu-day MA rule     %11.4f%% %11.4f%%\n", n,
                100 * investm::stats::Mean(r), 100 * investm::stats::StdDev(r));
  }
  std::printf(
      "  (prices follow a random walk here, so any edge is luck;\n"
      "   try other seeds)\n");
}

}  // namespace

int main() {
  RatioAnalysis();
  Technical();
  return 0;
}
