#include "cpp/ch03_security_markets/costs.h"

#include <algorithm>
#include <cmath>

#include "cpp/core/check.h"

namespace investm::ch03 {

double CommissionSchedule::Commission(double shares, double price) const {
  const double c =
      per_ticket + per_share * shares + fraction_of_principal * shares * price;
  return std::max(c, minimum);
}

double EffectiveHalfSpread(double trade_price, const Quote& quote) {
  return std::fabs(trade_price - quote.Mid()) / quote.Mid();
}

double SqrtPriceImpact(double shares, double average_daily_volume,
                       double daily_vol, double k) {
  INVESTM_CHECK(average_daily_volume > 0.0, "ADV must be positive");
  return k * daily_vol * std::sqrt(shares / average_daily_volume);
}

double OneWayCost(double shares, double price, const Quote& quote,
                  const CommissionSchedule& schedule, double impact_fraction) {
  const double principal = shares * price;
  INVESTM_CHECK(principal > 0.0, "principal must be positive");
  return schedule.Commission(shares, price) / principal +
         0.5 * quote.RelativeSpread() + impact_fraction;
}

}  // namespace investm::ch03
