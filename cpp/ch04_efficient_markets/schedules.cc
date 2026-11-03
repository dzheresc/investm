#include "cpp/ch04_efficient_markets/schedules.h"

#include <algorithm>

#include "cpp/core/check.h"
#include "cpp/core/root_find.h"

namespace investm::ch04 {

double DemandToBuy(std::span<const Investor> investors, double price,
                   double sensitivity) {
  INVESTM_CHECK(price > 0.0, "price must be positive");
  double q = 0.0;
  for (const Investor& i : investors) {
    if (i.value > price) {
      q += std::min(i.max_buy,
                    sensitivity * (i.value - price) / price * i.max_buy);
    }
  }
  return q;
}

double SupplyToSell(std::span<const Investor> investors, double price,
                    double sensitivity) {
  INVESTM_CHECK(price > 0.0, "price must be positive");
  double q = 0.0;
  for (const Investor& i : investors) {
    if (i.value < price) {
      q += std::min(i.shares_held,
                    sensitivity * (price - i.value) / price * i.shares_held);
    }
  }
  return q;
}

std::optional<double> EquilibriumPrice(std::span<const Investor> investors,
                                       double sensitivity) {
  if (investors.empty()) return std::nullopt;
  const auto [lo, hi] = std::minmax_element(
      investors.begin(), investors.end(),
      [](const Investor& a, const Investor& b) { return a.value < b.value; });
  if (lo->value >= hi->value) return std::nullopt;
  auto excess_demand = [&](double p) {
    return DemandToBuy(investors, p, sensitivity) -
           SupplyToSell(investors, p, sensitivity);
  };
  return Brent(excess_demand, lo->value, hi->value);
}

double ArcElasticity(double p1, double q1, double p2, double q2) {
  const double dq = (q2 - q1) / (0.5 * (q1 + q2));
  const double dp = (p2 - p1) / (0.5 * (p1 + p2));
  return dq / dp;
}

}  // namespace investm::ch04
