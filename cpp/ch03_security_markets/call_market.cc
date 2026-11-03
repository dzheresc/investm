#include "cpp/ch03_security_markets/call_market.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "cpp/core/check.h"

namespace investm::ch03 {

double DemandAt(std::span<const LimitOrder> book, double price) {
  double q = 0.0;
  for (const LimitOrder& o : book) {
    if (o.is_buy && o.limit >= price) q += o.quantity;
  }
  return q;
}

double SupplyAt(std::span<const LimitOrder> book, double price) {
  double q = 0.0;
  for (const LimitOrder& o : book) {
    if (!o.is_buy && o.limit <= price) q += o.quantity;
  }
  return q;
}

CallAuctionResult ClearCallMarket(std::span<const LimitOrder> book) {
  INVESTM_CHECK(!book.empty(), "empty order book");
  std::vector<double> prices;
  prices.reserve(book.size());
  for (const LimitOrder& o : book) prices.push_back(o.limit);
  std::sort(prices.begin(), prices.end());
  prices.erase(std::unique(prices.begin(), prices.end()), prices.end());

  double best_volume = -1.0;
  double best_imbalance = 0.0;
  double best_low = 0.0;
  double best_high = 0.0;
  for (double p : prices) {
    const double demand = DemandAt(book, p);
    const double supply = SupplyAt(book, p);
    const double volume = std::min(demand, supply);
    const double imbalance = demand - supply;
    if (volume > best_volume ||
        (volume == best_volume &&
         std::fabs(imbalance) < std::fabs(best_imbalance))) {
      best_volume = volume;
      best_imbalance = imbalance;
      best_low = best_high = p;
    } else if (volume == best_volume &&
               std::fabs(imbalance) == std::fabs(best_imbalance)) {
      best_high = p;
    }
  }
  const double price = 0.5 * (best_low + best_high);
  const double demand = DemandAt(book, price);
  const double supply = SupplyAt(book, price);
  return {price, std::min(demand, supply), demand - supply};
}

}  // namespace investm::ch03
