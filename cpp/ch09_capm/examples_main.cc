// Worked examples for Chapter 9. All numbers are illustrative.

#include <cmath>
#include <cstdio>
#include <vector>

#include "cpp/ch07_portfolio_analysis/efficient_set.h"
#include "cpp/ch08_riskfree_borrowing_lending/riskfree.h"
#include "cpp/ch09_capm/capm.h"
#include "cpp/core/table.h"

namespace {

using investm::Matrix;
using investm::PrintHeading;
using investm::PrintPercent;
using investm::Vector;
namespace ch09 = investm::ch09;

const char* kNames[] = {"Utility", "Bank", "Retail", "Tech", "Biotech"};
const Vector kSd{0.14, 0.20, 0.24, 0.32, 0.40};
Matrix Cov() {
  const Matrix corr({{1.00, 0.45, 0.40, 0.30, 0.20},
                     {0.45, 1.00, 0.55, 0.45, 0.30},
                     {0.40, 0.55, 1.00, 0.50, 0.35},
                     {0.30, 0.45, 0.50, 1.00, 0.55},
                     {0.20, 0.30, 0.35, 0.55, 1.00}});
  return investm::CovarianceFromCorrelation(kSd, corr);
}
// Market capitalization weights.
const Vector kMarket{0.15, 0.25, 0.20, 0.30, 0.10};

void EquilibriumReturns() {
  PrintHeading("9.2-9.3 Equilibrium: CML, SML and implied expected returns");
  const double rf = 0.03;
  const double premium = 0.055;  // E_M - rf
  const Matrix cov = Cov();
  const Vector mu = ch09::ImpliedExpectedReturns(cov, kMarket, rf, premium);
  const Vector betas = ch09::Betas(cov, kMarket);
  const double market_sd = std::sqrt(investm::QuadraticForm(kMarket, cov));
  std::printf("  rf = 3%%, E_M - rf = 5.5%%, sigma_M = %.2f%%\n",
              100 * market_sd);
  std::printf("  %-8s %7s %8s %8s %10s %10s\n", "stock", "beta", "E %", "sd %",
              "mkt var %", "own var %");
  for (std::size_t i = 0; i < mu.size(); ++i) {
    const auto part =
        ch09::PartitionRisk(kSd[i] * kSd[i], betas[i], market_sd * market_sd);
    std::printf("  %-8s %7.3f %8.3f %8.2f %10.1f %10.1f\n", kNames[i], betas[i],
                100 * mu[i], 100 * kSd[i],
                100 * part.market_variance / (kSd[i] * kSd[i]),
                100 * part.nonmarket_variance / (kSd[i] * kSd[i]));
  }
  const auto t = investm::ch08::TangencyPortfolio(mu, cov, rf).value();
  std::printf("  tangency portfolio recovered from these returns:");
  for (double w : t.weights) std::printf(" %.3f", w);
  std::printf("\n  (= the market weights: separation theorem, 9.2.1)\n");
  std::printf("  CML: a portfolio with sd 10%% earns %.3f%%\n",
              100 * ch09::CapitalMarketLine(rf, rf + premium, market_sd, 0.10));
}

void MispricedSecurity() {
  PrintHeading("9.3 Alpha: a stock off the SML");
  const double rf = 0.03;
  const double e_m = 0.085;
  const double beta = 1.3;
  const double analyst_forecast = 0.115;
  PrintPercent("SML required return", ch09::SecurityMarketLine(rf, e_m, beta));
  PrintPercent("analyst's expected return", analyst_forecast);
  PrintPercent("alpha (underpriced if > 0)",
               ch09::Alpha(analyst_forecast, rf, e_m, beta));
}

void ZeroBeta() {
  PrintHeading("Appendix A.1 No riskfree borrowing: the zero-beta CAPM");
  const Matrix cov = Cov();
  const Vector mu =
      ch09::ImpliedExpectedReturns(cov, kMarket, 0.03, 0.055);  // Any mu.
  const auto f = investm::ch07::UnconstrainedFrontier::Create(mu, cov).value();
  const double e_m = 0.10;
  const double e_z = ch09::ZeroBetaReturn(f.a(), f.b(), f.c(), e_m);
  PrintPercent("market (frontier) expected return", e_m);
  PrintPercent("zero-beta portfolio expected return", e_z);
  for (double beta : {0.0, 0.5, 1.0, 1.5}) {
    std::printf("  beta %.1f -> E = %.3f%%\n", beta,
                100 * ch09::ZeroBetaSecurityMarketLine(e_z, e_m, beta));
  }
}

}  // namespace

int main() {
  EquilibriumReturns();
  MispricedSecurity();
  ZeroBeta();
  return 0;
}
