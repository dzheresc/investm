// Worked examples for Chapter 6. All numbers are illustrative.

#include <cstdio>
#include <random>
#include <vector>

#include "cpp/ch06_portfolio_selection/portfolio.h"
#include "cpp/ch06_portfolio_selection/risk_measures.h"
#include "cpp/ch06_portfolio_selection/utility.h"
#include "cpp/core/stats.h"
#include "cpp/core/table.h"

namespace {

using investm::Matrix;
using investm::PrintHeading;
using investm::PrintPercent;
using investm::PrintValue;
namespace ch06 = investm::ch06;

void TerminalWealth() {
  PrintHeading("6.1 Initial and terminal wealth");
  const std::vector<double> shares{150, 300, 80};
  const std::vector<double> prices{30, 22, 75};
  const std::vector<double> end{33, 21, 84};  // Includes any dividends.
  const auto w = ch06::WeightsFromHoldings(shares, prices);
  std::printf("  weights: %.4f %.4f %.4f\n", w[0], w[1], w[2]);
  PrintPercent("portfolio return (W1 - W0) / W0",
               ch06::PortfolioReturnFromPrices(shares, prices, end));
}

void UtilityAndRiskPremiums() {
  PrintHeading("6.3 Utility, certainty equivalents and risk premiums");
  std::printf("  gamble: $70,000 or $130,000 with equal probability\n");
  const std::vector<double> wealth{70000, 130000};
  const std::vector<double> probs{0.5, 0.5};
  const struct {
    const char* name;
    ch06::Utility u;
  } investors[] = {
      {"risk neutral, U = W", ch06::Utility(ch06::UtilityKind::kLinear)},
      {"log utility", ch06::Utility(ch06::UtilityKind::kLog)},
      {"power, g = 4", ch06::Utility(ch06::UtilityKind::kPower, 4.0)},
      {"exponential, a = 1e-4",
       ch06::Utility(ch06::UtilityKind::kExponential, 1e-4)},
      {"risk seeking, U = W^2", ch06::Utility(ch06::UtilityKind::kConvex)},
  };
  investm::Table table({"investor", "CE $", "premium $"}, 22);
  for (const auto& inv : investors) {
    table.AddRow({inv.name,
                  std::to_string(static_cast<int>(
                      ch06::CertaintyEquivalent(inv.u, wealth, probs))),
                  std::to_string(static_cast<int>(
                      ch06::RiskPremium(inv.u, wealth, probs)))});
  }
  table.Print();
}

void IndifferenceCurves() {
  PrintHeading("6.4 Indifference curves (U = E - sigma^2 / tau)");
  const std::vector<double> sigmas{0.0, 0.05, 0.10, 0.15, 0.20, 0.25};
  investm::Table table({"sigma %", "tau=0.25", "tau=0.50", "tau=1.00"});
  const auto low = ch06::IndifferenceCurve(0.06, sigmas, 0.25);
  const auto mid = ch06::IndifferenceCurve(0.06, sigmas, 0.50);
  const auto high = ch06::IndifferenceCurve(0.06, sigmas, 1.00);
  for (std::size_t i = 0; i < sigmas.size(); ++i) {
    table.AddRow({100 * sigmas[i], 100 * low[i], 100 * mid[i], 100 * high[i]},
                 2);
  }
  table.Print();
  std::printf("  Required expected return (%%) for the same 6%% utility.\n");
}

void PortfolioMoments() {
  PrintHeading("6.5 Expected return and standard deviation of a portfolio");
  const std::vector<double> mu{0.08, 0.12, 0.15};
  const std::vector<double> sd{0.15, 0.22, 0.30};
  const Matrix corr({{1.0, 0.3, 0.2}, {0.3, 1.0, 0.4}, {0.2, 0.4, 1.0}});
  const Matrix cov = investm::CovarianceFromCorrelation(sd, corr);
  const std::vector<double> w{0.5, 0.3, 0.2};
  std::printf("  three stocks, weights 50/30/20\n");
  PrintPercent("expected return", ch06::PortfolioExpectedReturn(w, mu));
  PrintPercent("standard deviation", ch06::PortfolioStdDev(w, cov));
  PrintPercent("weighted average of the standard deviations",
               ch06::PortfolioExpectedReturn(w, sd));
  std::printf("  two assets, s1 = 20%%, s2 = 30%%, 60/40 mix:\n");
  for (double rho : {1.0, 0.5, 0.0, -0.5, -1.0}) {
    std::printf("    rho = %+.1f  sigma_P = %6.3f%%\n", rho,
                100 * ch06::TwoAssetStdDev(0.6, 0.20, 0.30, rho));
  }
}

void AlternativeRiskMeasures() {
  PrintHeading("Institutional issues: alternative risk measures");
  std::mt19937_64 rng(11);
  std::student_t_distribution<double> fat(4.0);
  std::vector<double> r;
  for (int i = 0; i < 5000; ++i) r.push_back(0.008 + 0.03 * fat(rng));
  const double m = investm::stats::Mean(r);
  const double s = investm::stats::StdDev(r);
  PrintPercent("mean monthly return", m);
  PrintPercent("standard deviation", s);
  PrintPercent("downside deviation below 0", ch06::DownsideDeviation(r, 0.0));
  PrintPercent("probability of a loss", ch06::ShortfallProbability(r, 0.0));
  PrintPercent("99% VaR, historical", ch06::HistoricalVaR(r, 0.99));
  PrintPercent("99% VaR, normal approximation", ch06::NormalVaR(m, s, 0.99));
  PrintPercent("99% expected shortfall, historical",
               ch06::HistoricalExpectedShortfall(r, 0.99));
  PrintPercent("99% expected shortfall, normal",
               ch06::NormalExpectedShortfall(m, s, 0.99));
  std::printf(
      "  Fat tails make the normal approximation understate tail loss.\n");
}

}  // namespace

int main() {
  TerminalWealth();
  UtilityAndRiskPremiums();
  IndifferenceCurves();
  PortfolioMoments();
  AlternativeRiskMeasures();
  return 0;
}
