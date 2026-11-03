// Worked examples for Chapter 14. All numbers are illustrative.

#include <cstdio>
#include <vector>

#include "cpp/ch05_riskless_securities/cash_flows.h"
#include "cpp/ch14_bond_analysis/credit.h"
#include "cpp/ch14_bond_analysis/duration.h"
#include "cpp/core/regression.h"
#include "cpp/core/table.h"

namespace {

using investm::Matrix;
using investm::PrintHeading;
using investm::PrintPercent;
using investm::PrintValue;
using investm::Vector;
namespace ch05 = investm::ch05;
namespace ch14 = investm::ch14;

void PricingTheorems() {
  PrintHeading("14.2 Bond pricing theorems (yield starts at 5.5%)");
  std::printf("  %-18s %10s %10s %10s %10s\n", "bond", "-100bp", "-50bp",
              "+50bp", "+100bp");
  struct Spec {
    const char* name;
    double coupon;
    double years;
  };
  for (const Spec& s :
       {Spec{"3% 5-year", 0.03, 5}, Spec{"3% 15-year", 0.03, 15},
        Spec{"3% 30-year", 0.03, 30}, Spec{"9% 15-year", 0.09, 15},
        Spec{"zero 15-year", 0.0, 15}}) {
    const auto f = ch05::CouponBond(100.0, s.coupon, s.years, 2);
    const double p0 = ch05::PresentValueAtYield(f, 0.055, 2);
    std::printf("  %-18s", s.name);
    for (double dy : {-0.01, -0.005, 0.005, 0.01}) {
      std::printf(" %9.2f%%",
                  100 * (ch05::PresentValueAtYield(f, 0.055 + dy, 2) / p0 - 1));
    }
    std::printf("\n");
  }
}

void DurationAndConvexity() {
  PrintHeading("14.3 Duration and convexity of a 4.5% 12-year bond at 5.8%");
  const auto f = ch05::CouponBond(100.0, 0.045, 12.0, 2);
  const double y = 0.058;
  const double p = ch05::PresentValueAtYield(f, y, 2);
  const double d = ch14::ModifiedDuration(f, y, 2);
  const double c = ch14::Convexity(f, y, 2);
  PrintValue("price", p);
  PrintValue("Macaulay duration (years)", ch14::MacaulayDuration(f, y, 2));
  PrintValue("modified duration", d);
  PrintValue("convexity", c);
  PrintValue("PVBP (per 100 face)", ch14::PriceValueOfBasisPoint(f, y, 2));
  std::printf("  %8s %10s %12s %12s\n", "dy", "actual", "duration",
              "dur+convex");
  for (double dy : {-0.03, -0.01, 0.01, 0.03}) {
    std::printf("  %+7.2f%% %10.4f %12.4f %12.4f\n", 100 * dy,
                ch05::PresentValueAtYield(f, y + dy, 2) - p,
                ch14::ApproxPriceChange(p, d, 0.0, dy),
                ch14::ApproxPriceChange(p, d, c, dy));
  }
  PrintValue("duration of a perpetuity at 5.8%", ch14::PerpetuityDuration(y));
}

void DefaultRisk() {
  PrintHeading("14.4 Promised vs expected yield (6% 7-year, priced at 91)");
  std::printf("  %10s %10s %10s %10s %10s\n", "P(def)/yr", "promised",
              "expected", "def.prem", "risk prem");
  for (double p : {0.0, 0.01, 0.02, 0.04}) {
    const auto d =
        ch14::DecomposeYield(100.0, 0.06, 7, 91.0, p, 0.40, 0.042).value();
    std::printf("  %9.1f%% %9.3f%% %9.3f%% %9.3f%% %9.3f%%\n", 100 * p,
                100 * d.promised_yield, 100 * d.expected_yield,
                100 * d.default_premium, 100 * d.risk_premium);
  }
  PrintPercent("implied 1-yr default prob, 7.5% vs 4.2%, 40% recovery",
               ch14::ImpliedDefaultProbability(0.075, 0.042, 0.40));
}

void SpreadDeterminants() {
  PrintHeading("14.5 Yield spread regression on bond attributes");
  // Columns: years to maturity, debt / assets, log issue size.
  const Matrix x({{3, 0.30, 6.2},
                  {10, 0.45, 5.8},
                  {7, 0.60, 5.1},
                  {5, 0.25, 6.8},
                  {12, 0.55, 5.5},
                  {2, 0.70, 4.9},
                  {8, 0.35, 6.0},
                  {15, 0.40, 6.4},
                  {4, 0.65, 5.0},
                  {9, 0.50, 5.6}});
  const Vector spread{0.0072, 0.0151, 0.0208, 0.0058, 0.0196,
                      0.0225, 0.0105, 0.0149, 0.0221, 0.0163};
  const auto fit = investm::MultipleOls(x, spread).value();
  const char* names[] = {"intercept", "maturity", "debt/assets", "log size"};
  for (std::size_t i = 0; i < fit.coefficients.size(); ++i) {
    std::printf("  %-12s %+9.5f (se %.5f)\n", names[i], fit.coefficients[i],
                fit.std_errors[i]);
  }
  PrintValue("R^2", fit.r_squared);
}

void CreditScore() {
  PrintHeading("14.6 A discriminant credit score from two ratios");
  const Matrix sound(
      {{7.5, 0.32}, {5.9, 0.41}, {10.2, 0.22}, {6.6, 0.38}, {8.8, 0.29}});
  const Matrix distressed(
      {{1.4, 0.71}, {2.3, 0.62}, {0.9, 0.83}, {1.8, 0.66}, {2.6, 0.58}});
  const auto lda = ch14::LinearDiscriminant::Fit(sound, distressed).value();
  std::printf("  score = %.4f x coverage %+.4f x debt/assets, cutoff %.4f\n",
              lda.weights()[0], lda.weights()[1], lda.cutoff());
  for (const Vector& firm :
       {Vector{6.0, 0.35}, Vector{4.0, 0.45}, Vector{1.5, 0.40}}) {
    std::printf("  coverage %.1f, debt/assets %.2f -> score %7.3f : %s\n",
                firm[0], firm[1], lda.Score(firm),
                lda.IsGroupA(firm) ? "sound" : "distressed");
  }
}

}  // namespace

int main() {
  PricingTheorems();
  DurationAndConvexity();
  DefaultRisk();
  SpreadDeterminants();
  CreditScore();
  return 0;
}
