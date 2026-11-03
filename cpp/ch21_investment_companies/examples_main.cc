// Worked examples for Chapter 21. All numbers are illustrative.

#include <cstdio>
#include <vector>

#include "cpp/ch21_investment_companies/funds.h"
#include "cpp/core/table.h"

namespace {

using investm::PrintHeading;
using investm::PrintPercent;
using investm::PrintValue;
namespace ch21 = investm::ch21;

void NavAndClosedEnd() {
  PrintHeading("21.2-21.3 NAV and closed-end fund discounts");
  const double nav = ch21::NetAssetValue(812.5e6, 12.5e6, 40e6);
  PrintValue("NAV per share ($812.5m assets, $12.5m debt, 40m sh)", nav);
  for (double price : {16.80, 19.10, 21.40}) {
    std::printf("  market price %.2f -> %+.2f%% %s\n", price,
                100 * ch21::PremiumOrDiscount(price, nav),
                price < nav ? "discount" : "premium");
  }
}

void OpenEnd() {
  PrintHeading("21.4 Open-end funds: loads and distributions");
  PrintValue("offering price, NAV 24.70, 5.75% load",
             ch21::OfferingPrice(24.70, 0.0575));
  PrintPercent("load as a share of the amount invested",
               ch21::LoadAsFractionOfInvestment(0.0575));
  PrintPercent("return: NAV 24.70 -> 25.90, $0.35 income, $0.80 gains",
               ch21::FundReturn(24.70, 25.90, 0.35, 0.80));
  PrintValue("1,000 shares after reinvesting $1.15 at 25.90",
             ch21::SharesAfterReinvestment(1000, 1.15, 25.90));
}

void Fees() {
  PrintHeading("21.5 What fees cost: $10,000 for N years at 7% gross");
  const std::vector<double> cdsc{0.05, 0.04, 0.03, 0.02, 0.01};
  std::printf("  %5s %12s %12s %12s %12s\n", "years", "index 0.05%",
              "no-load 1%", "A: 5% + .5%", "B: CDSC+1.5%");
  for (int n : {1, 3, 5, 10, 20, 30}) {
    std::printf("  %5d %12.0f %12.0f %12.0f %12.0f\n", n,
                ch21::TerminalValue(1e4, 0.07, 0.0005, n, 0.0),
                ch21::TerminalValue(1e4, 0.07, 0.01, n, 0.0),
                ch21::TerminalValue(1e4, 0.07, 0.005, n, 0.05),
                ch21::TerminalValue(1e4, 0.07, 0.015, n, 0.0, cdsc));
  }
  std::printf(
      "  annualized drag of A: 1 yr %.2f%%, 5 yrs %.2f%%, "
      "20 yrs %.2f%%\n",
      100 * ch21::AnnualizedFeeDrag(0.07, 0.005, 1, 0.05),
      100 * ch21::AnnualizedFeeDrag(0.07, 0.005, 5, 0.05),
      100 * ch21::AnnualizedFeeDrag(0.07, 0.005, 20, 0.05));
  const auto y = ch21::BreakevenYears({0.005, 0.05}, {0.01, 0.0}, 0.07);
  if (y) {
    std::printf("  A beats the 1%% no-load after %d years\n", *y);
  }
}

}  // namespace

int main() {
  NavAndClosedEnd();
  OpenEnd();
  Fees();
  return 0;
}
