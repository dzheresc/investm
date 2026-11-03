#include "cpp/ch13_fixed_income/fixed_income.h"

#include <algorithm>
#include <cmath>
#include <limits>

#include "cpp/ch05_riskless_securities/cash_flows.h"
#include "cpp/core/check.h"
#include "cpp/core/root_find.h"

namespace investm::ch13 {
namespace {

int SerialDays(Date d) {
  return static_cast<int>(std::chrono::sys_days(d).time_since_epoch().count());
}

int Thirty360(Date a, Date b) {
  int d1 = static_cast<int>(static_cast<unsigned>(a.day()));
  int d2 = static_cast<int>(static_cast<unsigned>(b.day()));
  if (d1 == 31) d1 = 30;
  if (d2 == 31 && d1 == 30) d2 = 30;
  const int y = static_cast<int>(b.year()) - static_cast<int>(a.year());
  const int m = static_cast<int>(static_cast<unsigned>(b.month())) -
                static_cast<int>(static_cast<unsigned>(a.month()));
  return 360 * y + 30 * m + (d2 - d1);
}

int MonthsPerPeriod(int frequency) {
  INVESTM_CHECK(frequency > 0 && 12 % frequency == 0,
                "frequency must divide 12");
  return 12 / frequency;
}

}  // namespace

int DayCountDays(Date a, Date b, DayCount convention) {
  INVESTM_CHECK(a.ok() && b.ok(), "invalid date");
  INVESTM_CHECK(a <= b, "dates out of order");
  return convention == DayCount::kThirty360 ? Thirty360(a, b)
                                            : SerialDays(b) - SerialDays(a);
}

Date AddMonths(Date d, int months) {
  const Date moved = d + std::chrono::months(months);
  if (moved.ok()) return moved;
  return std::chrono::year_month_day_last(
      moved.year(), std::chrono::month_day_last(moved.month()));
}

CouponPeriod SurroundingCoupons(const Bond& bond, Date settlement) {
  INVESTM_CHECK(settlement < bond.maturity, "bond has matured");
  const int step = MonthsPerPeriod(bond.frequency);
  // k is the smallest count with AddMonths(maturity, -step * (k + 1)) <=
  // settlement. Start one period short of the whole months to maturity and
  // correct by a step or two, instead of walking back from maturity.
  const int months_left =
      12 * (static_cast<int>(bond.maturity.year()) -
            static_cast<int>(settlement.year())) +
      static_cast<int>(static_cast<unsigned>(bond.maturity.month())) -
      static_cast<int>(static_cast<unsigned>(settlement.month()));
  int k = std::max(0, months_left / step - 1);
  while (k > 0 && AddMonths(bond.maturity, -step * k) <= settlement) --k;
  while (AddMonths(bond.maturity, -step * (k + 1)) > settlement) ++k;
  return {AddMonths(bond.maturity, -step * (k + 1)),
          AddMonths(bond.maturity, -step * k), k + 1};
}

double AccruedInterest(const Bond& bond, Date settlement) {
  const CouponPeriod p = SurroundingCoupons(bond, settlement);
  const double coupon = bond.face * bond.coupon_rate / bond.frequency;
  const double period_days =
      bond.day_count == DayCount::kThirty360
          ? 360.0 / bond.frequency
          : DayCountDays(p.previous, p.next, bond.day_count);
  return coupon * DayCountDays(p.previous, settlement, bond.day_count) /
         period_days;
}

double DirtyPrice(const Bond& bond, Date settlement, double yield) {
  const CouponPeriod p = SurroundingCoupons(bond, settlement);
  const double coupon = bond.face * bond.coupon_rate / bond.frequency;
  const double period_days =
      bond.day_count == DayCount::kThirty360
          ? 360.0 / bond.frequency
          : DayCountDays(p.previous, p.next, bond.day_count);
  const double w =
      DayCountDays(settlement, p.next, bond.day_count) / period_days;
  const double g = 1.0 + yield / bond.frequency;
  INVESTM_CHECK(g > 0.0, "yield below -100% per period");
  double pv = 0.0;
  for (int k = 0; k < p.remaining; ++k) {
    double flow = coupon;
    if (k + 1 == p.remaining) flow += bond.face;
    pv += flow / std::pow(g, k + w);
  }
  return pv;
}

double CleanPrice(const Bond& bond, Date settlement, double yield) {
  return DirtyPrice(bond, settlement, yield) -
         AccruedInterest(bond, settlement);
}

std::optional<double> YieldFromCleanPrice(const Bond& bond, Date settlement,
                                          double clean_price) {
  INVESTM_CHECK(clean_price > 0.0, "price must be positive");
  const double target = clean_price + AccruedInterest(bond, settlement);
  auto f = [&](double y) { return DirtyPrice(bond, settlement, y) - target; };
  const double lo = -0.99 * bond.frequency;
  return BrentExpandUp(f, lo, 1.0);
}

double PriceFrom32nds(int handle, double thirty_seconds) {
  INVESTM_CHECK(thirty_seconds >= 0.0 && thirty_seconds < 32.0,
                "32nds must be in [0, 32)");
  return handle + thirty_seconds / 32.0;
}

std::optional<double> YieldToCall(double face, double coupon_rate,
                                  const CallDate& call, double price,
                                  int frequency) {
  INVESTM_CHECK(call.time > 0.0, "call date must be in the future");
  ch05::CashFlows flows =
      ch05::CouponBond(face, coupon_rate, call.time, frequency);
  flows.back().amount += call.price - face;
  return ch05::YieldToMaturity(flows, price, frequency);
}

std::optional<YieldToWorstResult> YieldToWorst(double face, double coupon_rate,
                                               double years_to_maturity,
                                               std::span<const CallDate> calls,
                                               double price, int frequency) {
  const auto ytm = ch05::YieldToMaturity(
      ch05::CouponBond(face, coupon_rate, years_to_maturity, frequency), price,
      frequency);
  if (!ytm) return std::nullopt;
  YieldToWorstResult worst{*ytm, years_to_maturity, true};
  for (const CallDate& c : calls) {
    if (c.time >= years_to_maturity) continue;
    const auto y = YieldToCall(face, coupon_rate, c, price, frequency);
    if (!y) return std::nullopt;
    if (*y < worst.yield - 1e-12) worst = {*y, c.time, false};
  }
  return worst;
}

double PerpetuityValue(double dividend, double required_return) {
  INVESTM_CHECK(required_return > 0.0, "required return must be positive");
  return dividend / required_return;
}

double PerpetuityYield(double dividend, double price) {
  INVESTM_CHECK(price > 0.0, "price must be positive");
  return dividend / price;
}

}  // namespace investm::ch13
