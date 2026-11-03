#include "cpp/ch02_buying_and_selling/orders.h"

#include "cpp/core/check.h"

namespace investm::ch02 {
namespace {

bool LimitSatisfied(Side side, double price, double limit) {
  return side == Side::kBuy ? price <= limit : price >= limit;
}

bool StopTriggered(Side side, double price, double stop) {
  return side == Side::kBuy ? price >= stop : price <= stop;
}

}  // namespace

std::optional<Fill> Simulate(const Order& order, std::span<const double> path) {
  INVESTM_CHECK(!path.empty(), "empty price path");
  bool active =
      order.type == OrderType::kMarket || order.type == OrderType::kLimit;
  for (std::size_t i = 0; i < path.size(); ++i) {
    const double price = path[i];
    if (!active) {
      if (!StopTriggered(order.side, price, order.stop)) continue;
      active = true;
    }
    switch (order.type) {
      case OrderType::kMarket:
      case OrderType::kStop:
        return Fill{i, price};
      case OrderType::kLimit:
      case OrderType::kStopLimit:
        if (LimitSatisfied(order.side, price, order.limit)) {
          return Fill{i, price};
        }
        break;
    }
  }
  return std::nullopt;
}

}  // namespace investm::ch02
