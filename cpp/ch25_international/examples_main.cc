// Worked examples for Chapter 25. All numbers are illustrative.

#include <cmath>
#include <cstdio>
#include <utility>
#include <vector>

#include "cpp/ch25_international/international.h"
#include "cpp/core/table.h"

namespace {

using investm::PrintHeading;
using investm::PrintPercent;
using investm::PrintValue;
namespace ch25 = investm::ch25;

void CurrencyRisk() {
  PrintHeading("25.3 Returns in domestic currency");
  std::printf("  foreign stock returns 9%% in local currency\n");
  for (auto [s0, s1] :
       {std::pair{1.08, 1.17}, std::pair{1.08, 1.08}, std::pair{1.08, 0.97}}) {
    const double e = ch25::CurrencyReturn(s0, s1);
    std::printf("  spot %.2f -> %.2f (%+.2f%%): domestic return %+.2f%%\n", s0,
                s1, 100 * e, 100 * ch25::DomesticReturn(0.09, e));
  }
  for (double rho : {-0.3, 0.0, 0.3}) {
    std::printf("  sd local 18%%, sd currency 10%%, rho %+.1f -> sd %.2f%%\n",
                rho,
                100 * std::sqrt(ch25::DomesticReturnVariance(0.18, 0.10, rho)));
  }
}

void Parity() {
  PrintHeading("25.4 Interest rate parity and hedging");
  const double s = 1.08;
  const double f = ch25::ParityForward(s, 0.045, 0.025, 1.0);
  PrintValue("1-year forward (4.5% home, 2.5% abroad)", f);
  PrintPercent("forward premium", ch25::ForwardPremium(s, f));
  std::printf("  foreign stock: expected 8%%, realized 11%%\n");
  for (double s1 : {0.97, 1.08, 1.17}) {
    std::printf("  spot ends %.2f: unhedged %+6.2f%%, hedged %+6.2f%%\n", s1,
                100 * ch25::DomesticReturn(0.11, ch25::CurrencyReturn(s, s1)),
                100 * ch25::HedgedReturn(0.11, 0.08, s, f, s1));
  }
}

void Ppp() {
  PrintHeading("25.5 Purchasing power parity");
  PrintPercent("expected change, 4% home vs 1.5% foreign inflation",
               ch25::PppExpectedChange(0.04, 0.015));
  std::printf("  real exchange rate: %.4f -> %.4f\n",
              ch25::RealExchangeRate(1.08, 100.0, 100.0),
              ch25::RealExchangeRate(1.02, 101.5, 104.0));
}

void Diversification() {
  PrintHeading("25.6 Diversifying abroad (home sd 15%, foreign sd 19%)");
  std::printf("  %8s", "foreign");
  for (double rho : {0.2, 0.5, 0.8}) std::printf("   rho %.1f", rho);
  std::printf("\n");
  for (double w = 0.0; w <= 1.0001; w += 0.2) {
    std::printf("  %7.0f%%", 100 * w);
    for (double rho : {0.2, 0.5, 0.8}) {
      std::printf(" %8.2f%%", 100 * ch25::TwoMarketStdDev(w, 0.15, 0.19, rho));
    }
    std::printf("\n");
  }
  for (double rho : {0.2, 0.5, 0.8}) {
    const double w = ch25::MinimumRiskForeignWeight(0.15, 0.19, rho);
    std::printf("  rho %.1f: minimum risk at %.1f%% abroad (sd %.2f%%)\n", rho,
                100 * w, 100 * ch25::TwoMarketStdDev(w, 0.15, 0.19, rho));
  }
}

}  // namespace

int main() {
  CurrencyRisk();
  Parity();
  Ppp();
  Diversification();
  return 0;
}
