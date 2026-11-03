#include "cpp/ch02_buying_and_selling/margin.h"

#include "gtest/gtest.h"

namespace investm::ch02 {
namespace {

TEST(MarginPurchaseTest, Arithmetic) {
  const MarginPurchase mp{100, 100.0, 0.60};
  EXPECT_DOUBLE_EQ(mp.Loan(), 4000.0);
  EXPECT_NEAR(mp.ActualMargin(100.0), 0.60, 1e-15);
  const double call = mp.MarginCallPrice(0.30);
  EXPECT_NEAR(call, 4000.0 / 70.0, 1e-12);
  EXPECT_NEAR(mp.ActualMargin(call), 0.30, 1e-12);
  EXPECT_NEAR(mp.CashToRestore(100.0, 0.6), 0.0, 1e-12);
  // At 50: value 5000, required equity 3000, actual equity 1000.
  EXPECT_NEAR(mp.CashToRestore(50.0, 0.6), 2000.0, 1e-9);
  // +10% price with a 10% loan rate: (1000 - 400) / 6000.
  EXPECT_NEAR(mp.ReturnOnEquity(110.0, 0.0, 0.10), 0.10, 1e-12);
}

TEST(ShortSaleTest, Arithmetic) {
  const ShortSale ss{100, 100.0, 0.50};
  EXPECT_NEAR(ss.ActualMargin(100.0), 0.50, 1e-15);
  const double call = ss.MarginCallPrice(0.30);
  EXPECT_NEAR(call, 15000.0 / 130.0, 1e-12);
  EXPECT_NEAR(ss.ActualMargin(call), 0.30, 1e-12);
  // Covered at 80 after paying a $1 dividend: 1900 gain on a 5000 deposit.
  EXPECT_NEAR(ss.ReturnOnEquity(80.0, 1.0), 0.38, 1e-12);
}

TEST(AccountTest, HedgedAccountEquityIgnoresParallelMoves) {
  // Margin loan, short proceeds and short deposit.
  Account account({{100, 50.0}, {-100, 50.0}}, -2500.0 + 5000.0 + 2500.0);
  EXPECT_DOUBLE_EQ(account.Equity(), 5000.0);
  EXPECT_DOUBLE_EQ(account.ActualMargin(), 0.5);
  account.set_positions({{100, 40.0}, {-100, 40.0}});
  EXPECT_DOUBLE_EQ(account.Equity(), 5000.0);
}

}  // namespace
}  // namespace investm::ch02
