#include "cpp/ch02_buying_and_selling/orders.h"

#include <vector>

#include "gtest/gtest.h"

namespace investm::ch02 {
namespace {

const std::vector<double> kPath{50.0, 49.2, 48.5, 47.8, 49.0, 51.5, 52.4, 50.9};

TEST(OrdersTest, MarketOrderFillsImmediately) {
  const auto fill = Simulate({Side::kBuy, OrderType::kMarket}, kPath);
  ASSERT_TRUE(fill.has_value());
  EXPECT_EQ(fill->index, 0u);
}

TEST(OrdersTest, LimitBuyWaitsForPrice) {
  const auto fill = Simulate({Side::kBuy, OrderType::kLimit, 48.0}, kPath);
  ASSERT_TRUE(fill.has_value());
  EXPECT_EQ(fill->index, 3u);
  EXPECT_DOUBLE_EQ(fill->price, 47.8);
  EXPECT_FALSE(Simulate({Side::kBuy, OrderType::kLimit, 47.0}, kPath));
}

TEST(OrdersTest, StopOrdersTriggerOnTheRightSide) {
  const auto stop_loss =
      Simulate({Side::kSell, OrderType::kStop, 0.0, 48.0}, kPath);
  ASSERT_TRUE(stop_loss.has_value());
  EXPECT_EQ(stop_loss->index, 3u);
  const auto stop_buy =
      Simulate({Side::kBuy, OrderType::kStop, 0.0, 51.0}, kPath);
  ASSERT_TRUE(stop_buy.has_value());
  EXPECT_DOUBLE_EQ(stop_buy->price, 51.5);
}

TEST(OrdersTest, StopLimitBecomesLimitOrderOnceTriggered) {
  // Triggered at 51.5, which is within the 51.5 limit.
  const auto wide =
      Simulate({Side::kBuy, OrderType::kStopLimit, 51.5, 51.0}, kPath);
  ASSERT_TRUE(wide.has_value());
  EXPECT_EQ(wide->index, 5u);
  // Triggered at 51.5, but only 50.9 satisfies a 51.0 limit.
  const auto tight =
      Simulate({Side::kBuy, OrderType::kStopLimit, 51.0, 51.0}, kPath);
  ASSERT_TRUE(tight.has_value());
  EXPECT_EQ(tight->index, 7u);
}

}  // namespace
}  // namespace investm::ch02
