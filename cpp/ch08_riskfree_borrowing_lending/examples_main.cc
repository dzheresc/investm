// Worked examples for Chapter 8. All numbers are illustrative.

#include <cstdio>
#include <vector>

#include "cpp/ch08_riskfree_borrowing_lending/riskfree.h"
#include "cpp/core/table.h"

namespace {

using investm::Matrix;
using investm::PrintHeading;
using investm::PrintPercent;
using investm::Vector;
namespace ch08 = investm::ch08;

const Vector kMu{0.045, 0.065, 0.085, 0.095, 0.110};
Matrix Cov() {
  const Matrix corr({{1.00, 0.40, 0.10, 0.05, 0.00},
                     {0.40, 1.00, 0.35, 0.30, 0.20},
                     {0.10, 0.35, 1.00, 0.75, 0.60},
                     {0.05, 0.30, 0.75, 1.00, 0.65},
                     {0.00, 0.20, 0.60, 0.65, 1.00}});
  return investm::CovarianceFromCorrelation(
      Vector{0.06, 0.10, 0.16, 0.20, 0.26}, corr);
}

void PrintPortfolio(const char* label, const investm::ch07::PortfolioPoint& p,
                    double rf) {
  std::printf("  %-22s E=%6.2f%% sd=%6.2f%% Sharpe=%.4f  w:", label,
              100 * p.expected_return, 100 * p.std_dev,
              ch08::SharpeRatio(p.expected_return, p.std_dev, rf));
  for (double w : p.weights) std::printf(" %+6.3f", w);
  std::printf("\n");
}

void LendingAndBorrowingLine() {
  PrintHeading("8.2-8.3 Riskfree lending and borrowing with one portfolio");
  std::printf("  risky portfolio: E = 10%%, sd = 18%%; riskfree rate 4%%\n");
  std::printf("  %8s %8s %8s  %s\n", "x_risky", "E %", "sd %", "position");
  for (double x : {0.0, 0.25, 0.5, 0.75, 1.0, 1.25, 1.5}) {
    const auto c = ch08::CombineWithRiskfree(0.04, 0.10, 0.18, x);
    std::printf("  %8.2f %8.2f %8.2f  %s\n", x, 100 * c.expected_return,
                100 * c.std_dev,
                x < 1.0 ? "lending" : (x > 1.0 ? "borrowing" : "all risky"));
  }
}

void Tangency() {
  PrintHeading("8.4 The tangency portfolio and the optimal portfolio O*");
  const double rf = 0.035;
  const Matrix cov = Cov();
  const auto t = ch08::TangencyPortfolio(kMu, cov, rf).value();
  PrintPortfolio("T, short sales", t, rf);
  const auto tn = ch08::TangencyPortfolioNoShortSales(kMu, cov, rf);
  PrintPortfolio("T, no short sales", tn, rf);
  for (double tau : {0.1, 0.3, 0.6, 1.2}) {
    const double x = ch08::OptimalFractionInTangency(t, rf, tau);
    const auto c =
        ch08::CombineWithRiskfree(rf, t.expected_return, t.std_dev, x);
    std::printf(
        "  tau = %.1f: %5.1f%% in T, %+6.1f%% riskfree -> E=%5.2f%% "
        "sd=%5.2f%%\n",
        tau, 100 * x, 100 * (1 - x), 100 * c.expected_return, 100 * c.std_dev);
  }
}

void DifferentRates() {
  PrintHeading("Appendix A Lend at 3.0%, borrow at 4.5%");
  const char* names[] = {"lend", "risky only", "borrow"};
  for (double tau : {0.1, 0.3, 0.6, 1.0, 2.0}) {
    const auto c =
        ch08::OptimalWithDifferentRates(kMu, Cov(), 0.03, 0.045, tau).value();
    std::printf("  tau = %.1f  %-10s  risky %6.1f%%  E=%5.2f%% sd=%5.2f%%\n",
                tau, names[static_cast<int>(c.region)], 100 * c.fraction_risky,
                100 * c.expected_return, 100 * c.std_dev);
  }
}

}  // namespace

int main() {
  LendingAndBorrowingLine();
  Tangency();
  DifferentRates();
  return 0;
}
