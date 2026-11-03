#include "cpp/ch03_security_markets/call_market.h"

#include <vector>

#include "gtest/gtest.h"

namespace investm::ch03 {
namespace {

TEST(CallMarketTest, ClearsWhereVolumeIsLargest) {
  const std::vector<LimitOrder> book{
      {true, 30.50, 300},  {true, 30.25, 500},  {true, 30.00, 400},
      {true, 29.75, 600},  {false, 29.50, 200}, {false, 29.75, 300},
      {false, 30.00, 500}, {false, 30.25, 700},
  };
  // At 30.00 demand is 1200 and supply 1000, so 1000 shares trade.
  const CallAuctionResult r = ClearCallMarket(book);
  EXPECT_DOUBLE_EQ(r.price, 30.00);
  EXPECT_DOUBLE_EQ(r.volume, 1000.0);
  EXPECT_DOUBLE_EQ(r.imbalance, 200.0);
}

TEST(CallMarketTest, NonCrossingBookTradesNothing) {
  const std::vector<LimitOrder> book{{true, 10.0, 100}, {false, 11.0, 100}};
  const CallAuctionResult r = ClearCallMarket(book);
  EXPECT_DOUBLE_EQ(r.volume, 0.0);
  EXPECT_DOUBLE_EQ(r.price, 10.5);
}

}  // namespace
}  // namespace investm::ch03
