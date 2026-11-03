#include "cpp/ch05_riskless_securities/cash_flows.h"

#include <cmath>
#include <cstdint>

#include "cpp/core/check.h"
#include "cpp/core/root_find.h"

namespace investm::ch05 {

CashFlows CouponBond(double face, double coupon_rate, double years,
                     int frequency) {
  INVESTM_CHECK(frequency > 0 && years > 0.0, "invalid bond terms");
  const int64_t n = std::llround(years * frequency);
  const double coupon = face * coupon_rate / frequency;
  CashFlows flows;
  flows.reserve(static_cast<std::size_t>(n));
  for (int64_t k = 1; k <= n; ++k) {
    flows.push_back(
        {static_cast<double>(k) / frequency, coupon + (k == n ? face : 0.0)});
  }
  return flows;
}

CashFlows ZeroCoupon(double face, double years) { return {{years, face}}; }

double PresentValueAtYield(const CashFlows& flows, double yield,
                           int frequency) {
  INVESTM_CHECK(frequency > 0, "frequency must be positive");
  // log1p/exp keeps precision for small yields and avoids repeated pow().
  const double log_growth = std::log1p(yield / frequency) * frequency;
  double pv = 0.0;
  for (const CashFlow& cf : flows)
    pv += cf.amount * std::exp(-log_growth * cf.time);
  return pv;
}

double PresentValueContinuous(const CashFlows& flows, double yield) {
  double pv = 0.0;
  for (const CashFlow& cf : flows) pv += cf.amount * std::exp(-yield * cf.time);
  return pv;
}

std::optional<double> YieldToMaturity(const CashFlows& flows, double price,
                                      int frequency) {
  INVESTM_CHECK(price > 0.0, "price must be positive");
  auto pricing_error = [&](double y) {
    return PresentValueAtYield(flows, y, frequency) - price;
  };
  return BrentExpandUp(pricing_error, -0.99 * frequency, 0.20);
}

std::optional<double> InternalRateOfReturn(const CashFlows& flows, double lo,
                                           double hi) {
  auto npv = [&](double r) { return PresentValueAtYield(flows, r, 1); };
  return Brent(npv, lo, hi);
}

}  // namespace investm::ch05
