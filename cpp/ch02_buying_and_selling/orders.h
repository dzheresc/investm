// Chapter 2, Buying and Selling Securities.
// Section 2.3: types of orders (market, limit, stop, stop limit).
//
// A toy execution simulator. Given a sequence of trade prices, it reports
// where an order would have filled, if at all. Queue priority and partial
// fills are ignored; the point is the trigger logic of each order type.

#ifndef CPP_CH02_BUYING_AND_SELLING_ORDERS_H_
#define CPP_CH02_BUYING_AND_SELLING_ORDERS_H_

#include <cstddef>
#include <optional>
#include <span>

namespace investm::ch02 {

enum class Side { kBuy, kSell };
enum class OrderType { kMarket, kLimit, kStop, kStopLimit };

struct Order {
  Side side = Side::kBuy;
  OrderType type = OrderType::kMarket;
  double limit = 0.0;  // Used by kLimit and kStopLimit.
  double stop = 0.0;   // Used by kStop and kStopLimit.
};

struct Fill {
  std::size_t index;  // Position in the price path where the order filled.
  double price;
};

// Walks `path` with a good-till-cancelled order. Buy stops sit above the
// market and trigger on a rise; sell stops (stop-loss orders) sit below it and
// trigger on a fall. A triggered stop becomes a market order; a triggered
// stop limit becomes a limit order.
std::optional<Fill> Simulate(const Order& order, std::span<const double> path);

}  // namespace investm::ch02

#endif  // CPP_CH02_BUYING_AND_SELLING_ORDERS_H_
