// Chapter 3, Security Markets.
// Section 3.1.1: call markets. Orders are batched and executed together at a
// single price, the one that lets the most shares trade.

#ifndef CPP_CH03_SECURITY_MARKETS_CALL_MARKET_H_
#define CPP_CH03_SECURITY_MARKETS_CALL_MARKET_H_

#include <span>

namespace investm::ch03 {

struct LimitOrder {
  bool is_buy;
  double limit;  // Highest price a buyer pays, lowest price a seller accepts.
  double quantity;
};

// Shares demanded at `price`: all buy orders with limit >= price.
double DemandAt(std::span<const LimitOrder> book, double price);

// Shares offered at `price`: all sell orders with limit <= price.
double SupplyAt(std::span<const LimitOrder> book, double price);

struct CallAuctionResult {
  double price = 0.0;
  double volume = 0.0;     // Shares that change hands.
  double imbalance = 0.0;  // Demand minus supply at the clearing price.
};

// Tries every limit price in the book and keeps the one with the largest
// executable volume min(demand, supply). Ties go to the smallest absolute
// imbalance, then to the midpoint of the tied prices.
CallAuctionResult ClearCallMarket(std::span<const LimitOrder> book);

}  // namespace investm::ch03

#endif  // CPP_CH03_SECURITY_MARKETS_CALL_MARKET_H_
