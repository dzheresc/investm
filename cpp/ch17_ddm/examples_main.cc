// Worked examples for Chapter 17. All numbers are illustrative.

#include <cstdio>
#include <vector>

#include "cpp/ch17_ddm/ddm.h"
#include "cpp/core/table.h"

namespace {

using investm::PrintHeading;
using investm::PrintPercent;
using investm::PrintValue;
namespace ch17 = investm::ch17;

void Simple() {
  PrintHeading("17.3-17.4 Zero and constant growth");
  PrintValue("zero growth: $2.60 forever at 9.5%",
             ch17::ZeroGrowthValue(2.60, 0.095));
  std::printf("  constant growth, D0 = $1.80, k = 10%%\n");
  for (double g : {0.0, 0.02, 0.04, 0.06, 0.08}) {
    std::printf("    g = %3.0f%%  V = %8.2f\n", 100 * g,
                ch17::ConstantGrowthValue(1.80, g, 0.10));
  }
  PrintPercent("implied return at P = $36, D0 = $1.80, g = 5%",
               ch17::ConstantGrowthImpliedReturn(36.0, 1.80, 0.05));
}

void Multiple() {
  PrintHeading("17.5 Multiple growth: four explicit years, then 4.5%");
  const std::vector<double> d{0.90, 1.15, 1.40, 1.60};
  const double v = ch17::MultipleGrowthValue(d, 0.045, 0.105);
  PrintValue("value at k = 10.5%", v);
  PrintValue("NPV at a price of $24", ch17::NetPresentValue(v, 24.0));
  PrintPercent("implied return at $24",
               ch17::MultipleGrowthImpliedReturn(24.0, d, 0.045).value());
}

void ThreePhase() {
  PrintHeading("17.6 Three-phase model: 18% for 4 yrs, fading to 5% by yr 10");
  const auto d = ch17::ThreePhaseDividends(1.20, 0.18, 4, 10, 0.05);
  for (std::size_t t = 0; t < d.size(); ++t) {
    std::printf("  year %2zu  D = %6.3f\n", t + 1, d[t]);
  }
  for (double k : {0.09, 0.10, 0.11, 0.12}) {
    std::printf("  k = %4.1f%%  V = %8.2f\n", 100 * k,
                ch17::ThreePhaseValue(1.20, 0.18, 4, 10, 0.05, k));
  }
}

void PriceEarnings() {
  PrintHeading("17.7 P/E ratios, retention and growth opportunities");
  std::printf("  E1 = $4.00, k = 11%%\n");
  std::printf("  %8s %8s %8s %9s %8s %8s\n", "ROE", "retain", "g", "price",
              "P/E", "PVGO");
  for (double roe : {0.08, 0.11, 0.16}) {
    for (double b : {0.0, 0.4, 0.6}) {
      const double g = ch17::SustainableGrowth(roe, b);
      const double p = ch17::ValueWithReinvestment(4.0, b, roe, 0.11);
      std::printf("  %7.0f%% %7.0f%% %7.1f%% %9.2f %8.2f %8.2f\n", 100 * roe,
                  100 * b, 100 * g, p, p / 4.0,
                  ch17::PresentValueOfGrowthOpportunities(p, 4.0, 0.11));
    }
  }
  PrintValue("forward P/E, 45% payout, g 6%, k 10%",
             ch17::ForwardPriceEarnings(0.45, 0.06, 0.10));
}

}  // namespace

int main() {
  Simple();
  Multiple();
  ThreePhase();
  PriceEarnings();
  return 0;
}
