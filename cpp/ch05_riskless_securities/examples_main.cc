// Worked examples for Chapter 5. All numbers are illustrative.

#include <cstdio>
#include <vector>

#include "cpp/ch05_riskless_securities/cash_flows.h"
#include "cpp/ch05_riskless_securities/rates.h"
#include "cpp/ch05_riskless_securities/term_structure.h"
#include "cpp/core/table.h"

namespace {

using investm::PrintHeading;
using investm::PrintPercent;
using investm::PrintValue;
using investm::Table;
namespace ch05 = investm::ch05;

void RealRates() {
  PrintHeading("5.1 Nominal vs. real rates");
  PrintPercent("nominal 7%, inflation 3%: exact real rate",
               ch05::RealRate(0.07, 0.03));
  PrintPercent("approximation nominal - inflation",
               ch05::RealRateApprox(0.07, 0.03));
}

void YieldToMaturity() {
  PrintHeading("5.2 YTM of a 4-year 6% annual-coupon bond at $965");
  const auto bond = ch05::CouponBond(1000, 0.06, 4, 1);
  const double y = ch05::YieldToMaturity(bond, 965.0).value();
  PrintPercent("YTM", y);
  PrintValue("check: PV at YTM", ch05::PresentValueAtYield(bond, y), "$");
  const auto semi = ch05::CouponBond(1000, 0.06, 4, 2);
  PrintPercent("same bond paying semiannually (APR, m=2)",
               ch05::YieldToMaturity(semi, 965.0, 2).value());
}

ch05::SpotCurve BootstrapExample() {
  PrintHeading("5.3-5.6, 5.9 Spot rates, discount factors, forwards");
  const std::vector<ch05::BondQuote> quotes{
      {1000, 0.050, 1001.90}, {1000, 0.055, 999.10}, {1000, 0.060, 994.80},
      {1000, 0.065, 993.40},  {1000, 0.065, 979.70},
  };
  const ch05::SpotCurve curve = ch05::Bootstrap(quotes).value();
  Table table({"t", "price", "spot %", "d_t", "f(t-1,t) %", "par %"});
  for (int t = 1; t <= curve.max_maturity(); ++t) {
    table.AddRow({static_cast<double>(t),
                  quotes[static_cast<std::size_t>(t - 1)].price,
                  100.0 * curve.Spot(t), curve.DiscountFactor(t),
                  100.0 * curve.OneYearForward(t), 100.0 * curve.ParYield(t)});
  }
  table.Print();
  PrintPercent("2-year rate starting in 3 years, f(3,5)", curve.Forward(3, 5));

  PrintHeading("5.2 vs. 5.3 YTM is not the right discount rate");
  const auto bond = ch05::CouponBond(1000, 0.08, 5);
  const double fair = curve.PresentValue(bond);
  PrintValue("5-year 8% bond valued off the spot curve", fair, "$");
  PrintPercent("its YTM", ch05::YieldToMaturity(bond, fair).value());
  PrintPercent("5-year spot rate", curve.Spot(5));
  std::printf(
      "  YTM blends the spot rates with cash-flow weights, so same-maturity\n"
      "  bonds with different coupons have different YTMs.\n");
  return curve;
}

void Compounding() {
  PrintHeading("5.7 Compounding an 8% quoted annual rate");
  for (int m : {1, 2, 4, 12, 365}) {
    std::printf("  m = %3d     effective annual %.6f%%\n", m,
                100.0 * ch05::EffectiveAnnualRate(0.08, m));
  }
  std::printf("  continuous  effective annual %.6f%%\n",
              100.0 * ch05::EffectiveFromContinuous(0.08));
}

void BankDiscount() {
  PrintHeading("5.8 A 91-day T-bill quoted at a 4.80% discount");
  const double price = ch05::BillPriceFromDiscount(10000, 0.048, 91);
  PrintValue("price", price, "$");
  PrintPercent("bank discount yield",
               ch05::BankDiscountYield(10000, price, 91));
  PrintPercent("bond-equivalent yield",
               ch05::BondEquivalentYield(10000, price, 91));
  PrintPercent("money-market yield", ch05::MoneyMarketYield(10000, price, 91));
  PrintPercent("effective annual yield",
               ch05::BillEffectiveAnnualYield(10000, price, 91));
}

void TermStructureTheories(const ch05::SpotCurve& curve) {
  PrintHeading("5.10 Term structure theories");
  std::printf("  unbiased expectations: expected 1-year rates");
  for (double e : ch05::ExpectedFutureOneYearRates(curve)) {
    std::printf(" %.2f%%", 100.0 * e);
  }
  std::printf("\n");
  const ch05::SpotCurve lp = ch05::LiquidityPreferenceCurve(
      {0.05, 0.05, 0.05, 0.05, 0.05}, {0.0, 0.004, 0.007, 0.009, 0.010});
  std::printf(
      "  liquidity preference, flat 5%% expectations, rising premiums:\n   ");
  for (double s : lp.rates()) std::printf(" %.3f%%", 100.0 * s);
  std::printf("\n");

  PrintHeading("5.11 One-year holding-period returns on zeros");
  const ch05::SpotCurve next_unbiased = ch05::RollForwardOneYear(curve);
  const ch05::SpotCurve next_flat({0.06, 0.06, 0.06, 0.06});
  Table table({"maturity", "HPR fwd %", "HPR flat %"});
  for (int n = 1; n <= curve.max_maturity(); ++n) {
    table.AddRow({static_cast<double>(n),
                  100.0 * ch05::OneYearHprOfZero(curve, next_unbiased, n),
                  100.0 * ch05::OneYearHprOfZero(curve, next_flat, n)});
  }
  table.Print();
  std::printf(
      "  If the forwards come true, every maturity earns the 1-year rate.\n");
}

}  // namespace

int main() {
  RealRates();
  YieldToMaturity();
  const ch05::SpotCurve curve = BootstrapExample();
  Compounding();
  BankDiscount();
  TermStructureTheories(curve);
  return 0;
}
