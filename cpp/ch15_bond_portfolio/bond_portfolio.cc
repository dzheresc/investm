#include "cpp/ch15_bond_portfolio/bond_portfolio.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <vector>

#include "cpp/core/check.h"

namespace investm::ch15 {
namespace {

constexpr double kTimeTolerance = 1e-9;

int WholePeriods(double years, int frequency) {
  const double n = years * frequency;
  INVESTM_CHECK(std::abs(n - std::round(n)) < 1e-9,
                "time must be a whole number of periods");
  return static_cast<int>(std::lround(n));
}

}  // namespace

HorizonResult HorizonAnalysis(double face, double coupon_rate,
                              double years_to_maturity, double buy_yield,
                              double horizon, double end_yield,
                              double reinvest_rate, int frequency) {
  INVESTM_CHECK(frequency > 0, "frequency must be positive");
  INVESTM_CHECK(horizon > 0.0 && horizon <= years_to_maturity,
                "horizon must be within the bond's life");
  const int h = WholePeriods(horizon, frequency);
  const double buy = ch05::PresentValueAtYield(
      ch05::CouponBond(face, coupon_rate, years_to_maturity, frequency),
      buy_yield, frequency);
  const double remaining = years_to_maturity - horizon;
  const double sale =
      remaining > kTimeTolerance
          ? ch05::PresentValueAtYield(
                ch05::CouponBond(face, coupon_rate, remaining, frequency),
                end_yield, frequency)
          : face;
  const double coupon = face * coupon_rate / frequency;
  const double g = 1.0 + reinvest_rate / frequency;
  double accumulated = 0.0;  // Coupons plus reinvestment at the horizon.
  for (int k = 1; k <= h; ++k) accumulated += coupon * std::pow(g, h - k);
  const double coupons = coupon * h;
  const double end_value = sale + accumulated;
  HorizonResult r;
  r.purchase_price = buy;
  r.sale_price = sale;
  r.coupon_income = coupons;
  r.interest_on_interest = accumulated - coupons;
  r.total_return = end_value / buy - 1.0;
  r.realized_compound_yield =
      frequency * (std::pow(end_value / buy, 1.0 / h) - 1.0);
  return r;
}

double PortfolioDuration(std::span<const double> values,
                         std::span<const double> durations) {
  INVESTM_CHECK(values.size() == durations.size() && !values.empty(),
                "size mismatch");
  double total = 0.0;
  double weighted = 0.0;
  for (std::size_t i = 0; i < values.size(); ++i) {
    total += values[i];
    weighted += values[i] * durations[i];
  }
  INVESTM_CHECK(total != 0.0, "portfolio value is zero");
  return weighted / total;
}

std::optional<TwoBondHolding> ImmunizeTwoBonds(double liability_pv,
                                               double liability_duration,
                                               double d1, double d2) {
  INVESTM_CHECK(d1 < d2, "durations must differ, d1 < d2");
  if (liability_duration < d1 || liability_duration > d2) return std::nullopt;
  const double w2 = (liability_duration - d1) / (d2 - d1);
  return TwoBondHolding{liability_pv * (1.0 - w2), liability_pv * w2};
}

double Surplus(const CashFlows& assets, const CashFlows& liabilities,
               double yield, int frequency) {
  return ch05::PresentValueAtYield(assets, yield, frequency) -
         ch05::PresentValueAtYield(liabilities, yield, frequency);
}

CashFlows Aggregate(const std::vector<CashFlows>& bonds,
                    std::span<const double> units) {
  INVESTM_CHECK(bonds.size() == units.size(), "size mismatch");
  std::map<double, double> by_time;
  for (std::size_t b = 0; b < bonds.size(); ++b) {
    for (const auto& f : bonds[b]) by_time[f.time] += units[b] * f.amount;
  }
  CashFlows out;
  out.reserve(by_time.size());
  for (const auto& [t, a] : by_time) out.push_back({t, a});
  return out;
}

std::optional<std::vector<double>> CashMatch(
    const std::vector<CashFlows>& bonds, const CashFlows& liabilities) {
  CashFlows owed = liabilities;
  std::sort(owed.begin(), owed.end(),
            [](const auto& a, const auto& b) { return a.time < b.time; });
  std::vector<double> units(bonds.size(), 0.0);
  for (std::size_t i = owed.size(); i-- > 0;) {
    if (owed[i].amount <= 0.0) continue;
    std::size_t pick = bonds.size();
    for (std::size_t b = 0; b < bonds.size(); ++b) {
      INVESTM_CHECK(!bonds[b].empty(), "bond without cash flows");
      if (std::abs(bonds[b].back().time - owed[i].time) < kTimeTolerance) {
        pick = b;
        break;
      }
    }
    if (pick == bonds.size()) return std::nullopt;
    const double q = owed[i].amount / bonds[pick].back().amount;
    units[pick] += q;
    for (const auto& f : bonds[pick]) {
      for (auto& o : owed) {
        if (std::abs(o.time - f.time) < kTimeTolerance)
          o.amount -= q * f.amount;
      }
    }
  }
  return units;
}

double RequiredImmunizedAssets(double floor, double rate, double horizon,
                               int frequency) {
  INVESTM_CHECK(frequency > 0, "frequency must be positive");
  return floor / std::pow(1.0 + rate / frequency, frequency * horizon);
}

double ContingentCushion(double assets, double floor, double rate,
                         double horizon, int frequency) {
  return assets - RequiredImmunizedAssets(floor, rate, horizon, frequency);
}

double ParSwapRate(std::span<const double> discount_factors, double period) {
  INVESTM_CHECK(!discount_factors.empty() && period > 0.0, "bad schedule");
  double annuity = 0.0;
  for (double d : discount_factors) annuity += period * d;
  return (1.0 - discount_factors.back()) / annuity;
}

double PayFixedSwapValue(double notional, double fixed_rate,
                         std::span<const double> discount_factors,
                         double period) {
  INVESTM_CHECK(!discount_factors.empty() && period > 0.0, "bad schedule");
  double annuity = 0.0;
  for (double d : discount_factors) annuity += period * d;
  return notional * (1.0 - discount_factors.back()) -
         notional * fixed_rate * annuity;
}

}  // namespace investm::ch15
