#include "cpp/ch05_riskless_securities/term_structure.h"

#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

#include "cpp/core/check.h"

namespace investm::ch05 {

SpotCurve::SpotCurve(std::vector<double> spot_rates)
    : spot_(std::move(spot_rates)) {
  INVESTM_CHECK(!spot_.empty(), "spot curve needs at least one rate");
}

SpotCurve SpotCurve::FromDiscountFactors(const std::vector<double>& factors) {
  std::vector<double> spot;
  spot.reserve(factors.size());
  for (std::size_t i = 0; i < factors.size(); ++i) {
    INVESTM_CHECK(factors[i] > 0.0, "discount factors must be positive");
    spot.push_back(
        std::pow(1.0 / factors[i], 1.0 / static_cast<double>(i + 1)) - 1.0);
  }
  return SpotCurve(std::move(spot));
}

SpotCurve SpotCurve::FromForwards(
    const std::vector<double>& one_year_forwards) {
  std::vector<double> spot;
  spot.reserve(one_year_forwards.size());
  double growth = 1.0;
  for (std::size_t i = 0; i < one_year_forwards.size(); ++i) {
    growth *= 1.0 + one_year_forwards[i];
    spot.push_back(std::pow(growth, 1.0 / static_cast<double>(i + 1)) - 1.0);
  }
  return SpotCurve(std::move(spot));
}

double SpotCurve::Spot(int t) const {
  INVESTM_CHECK(t >= 1 && t <= max_maturity(), "maturity out of range");
  return spot_[static_cast<std::size_t>(t - 1)];
}

double SpotCurve::DiscountFactor(int t) const {
  if (t == 0) return 1.0;
  return std::pow(1.0 + Spot(t), -t);
}

double SpotCurve::Forward(int a, int b) const {
  INVESTM_CHECK(a >= 0 && b > a, "need 0 <= a < b");
  return std::pow(DiscountFactor(a) / DiscountFactor(b), 1.0 / (b - a)) - 1.0;
}

double SpotCurve::PresentValue(const CashFlows& flows) const {
  double pv = 0.0;
  for (const CashFlow& cf : flows) {
    const int64_t t = std::llround(cf.time);
    INVESTM_CHECK(std::fabs(cf.time - static_cast<double>(t)) < 1e-9,
                  "cash flows must fall on whole years");
    pv += cf.amount * DiscountFactor(static_cast<int>(t));
  }
  return pv;
}

double SpotCurve::ParYield(int t) const {
  double annuity = 0.0;
  for (int k = 1; k <= t; ++k) annuity += DiscountFactor(k);
  return (1.0 - DiscountFactor(t)) / annuity;
}

std::optional<SpotCurve> Bootstrap(const std::vector<BondQuote>& quotes) {
  INVESTM_CHECK(!quotes.empty(), "no bond quotes");
  std::vector<double> factors;
  factors.reserve(quotes.size());
  double annuity = 0.0;  // d_1 + ... + d_{n-1}.
  for (const BondQuote& q : quotes) {
    const double coupon = q.face * q.coupon_rate;
    const double d = (q.price - coupon * annuity) / (coupon + q.face);
    if (d <= 0.0) return std::nullopt;
    factors.push_back(d);
    annuity += d;
  }
  return SpotCurve::FromDiscountFactors(factors);
}

std::vector<double> ExpectedFutureOneYearRates(const SpotCurve& curve) {
  std::vector<double> expected;
  for (int t = 1; t <= curve.max_maturity(); ++t) {
    expected.push_back(curve.OneYearForward(t));
  }
  return expected;
}

SpotCurve LiquidityPreferenceCurve(const std::vector<double>& expected,
                                   const std::vector<double>& premiums) {
  INVESTM_CHECK(expected.size() == premiums.size(), "size mismatch");
  std::vector<double> forwards;
  forwards.reserve(expected.size());
  for (std::size_t i = 0; i < expected.size(); ++i) {
    forwards.push_back(expected[i] + premiums[i]);
  }
  return SpotCurve::FromForwards(forwards);
}

std::vector<double> ImpliedLiquidityPremiums(
    const SpotCurve& curve, const std::vector<double>& expected) {
  INVESTM_CHECK(
      expected.size() >= static_cast<std::size_t>(curve.max_maturity()),
      "need an expectation for every maturity");
  std::vector<double> premiums;
  for (int t = 1; t <= curve.max_maturity(); ++t) {
    premiums.push_back(curve.OneYearForward(t) -
                       expected[static_cast<std::size_t>(t - 1)]);
  }
  return premiums;
}

double HoldingPeriodReturn(double buy_price, double sell_price,
                           double coupons) {
  INVESTM_CHECK(buy_price > 0.0, "buy price must be positive");
  return (sell_price + coupons - buy_price) / buy_price;
}

double OneYearHprOfZero(const SpotCurve& today, const SpotCurve& next_year,
                        int n) {
  const double buy = today.DiscountFactor(n);
  const double sell = next_year.DiscountFactor(n - 1);
  return HoldingPeriodReturn(buy, sell);
}

SpotCurve RollForwardOneYear(const SpotCurve& curve) {
  INVESTM_CHECK(curve.max_maturity() >= 2, "need at least two maturities");
  std::vector<double> spot;
  for (int t = 1; t < curve.max_maturity(); ++t) {
    spot.push_back(curve.Forward(1, t + 1));
  }
  return SpotCurve(std::move(spot));
}

}  // namespace investm::ch05
