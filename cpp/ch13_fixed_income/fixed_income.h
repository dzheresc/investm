// Chapter 13, Fixed-Income Securities.
// Quoting and pricing conventions for coupon bonds traded between coupon
// dates (day counts, accrued interest, clean and dirty prices, 32nds),
// callable bonds (yield-to-call, yield-to-worst) and preferred stock.

#ifndef CPP_CH13_FIXED_INCOME_FIXED_INCOME_H_
#define CPP_CH13_FIXED_INCOME_FIXED_INCOME_H_

#include <chrono>
#include <optional>
#include <span>

namespace investm::ch13 {

using Date = std::chrono::year_month_day;

enum class DayCount {
  kActualActual,  // Treasuries: actual days over actual days in the period.
  kThirty360,     // US corporates and agencies: 30/360 (bond basis).
  kActual360,     // Money market.
  kActual365,
};

// Day count between two dates under a convention (b >= a).
int DayCountDays(Date a, Date b, DayCount convention);
// Adds whole months; a day past the end of the month clamps to month end.
Date AddMonths(Date d, int months);

// 13.3 A level-coupon bullet bond.
struct Bond {
  double face = 100.0;
  double coupon_rate = 0.0;  // Annual rate; face * rate / frequency paid.
  Date maturity;
  int frequency = 2;
  DayCount day_count = DayCount::kActualActual;
};

// The coupon dates that bracket a settlement date: previous < settle <= next
// (previous == settle on a coupon date). Coupon dates roll back from
// maturity in steps of 12 / frequency months.
struct CouponPeriod {
  Date previous;
  Date next;
  int remaining;  // Coupons still to be paid, including `next`.
};
CouponPeriod SurroundingCoupons(const Bond& bond, Date settlement);

// 13.3.2 Interest accrued since the last coupon, owed by the buyer.
double AccruedInterest(const Bond& bond, Date settlement);

// Street-convention pricing between coupon dates: each remaining flow k
// (k = 0, 1, ...) is discounted at (1 + y/f)^(k + w), where w is the
// fraction of the current period still to run. Prices are per bond.
double DirtyPrice(const Bond& bond, Date settlement, double yield);
double CleanPrice(const Bond& bond, Date settlement, double yield);
// Yield from a quoted (clean) price; nullopt if no yield reproduces it.
std::optional<double> YieldFromCleanPrice(const Bond& bond, Date settlement,
                                          double clean_price);

// Treasury quotes in 32nds: 99-16 means 99 + 16/32.
double PriceFrom32nds(int handle, double thirty_seconds);

// 13.6 Callable bonds. Times are in years from today and are whole numbers
// of coupon periods.
struct CallDate {
  double time;
  double price;  // Call price per `face`.
};
// Yield assuming the bond is called at `call.time` for `call.price`.
std::optional<double> YieldToCall(double face, double coupon_rate,
                                  const CallDate& call, double price,
                                  int frequency = 2);
struct YieldToWorstResult {
  double yield;
  double horizon;    // Years until the assumed redemption.
  bool is_maturity;  // True when the worst case is holding to maturity.
};
// Lowest of yield-to-maturity and the yields to every call date.
std::optional<YieldToWorstResult> YieldToWorst(double face, double coupon_rate,
                                               double years_to_maturity,
                                               std::span<const CallDate> calls,
                                               double price, int frequency = 2);

// 13.8 Preferred stock: a perpetuity of fixed dividends.
double PerpetuityValue(double dividend, double required_return);
double PerpetuityYield(double dividend, double price);

}  // namespace investm::ch13

#endif  // CPP_CH13_FIXED_INCOME_FIXED_INCOME_H_
