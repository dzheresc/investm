#include "cpp/ch16_common_stocks/corporate.h"

#include <vector>

#include "gtest/gtest.h"

namespace investm::ch16 {
namespace {

TEST(VotingTest, CumulativeVoting) {
  // 1,000 shares, 5 seats: 167 shares guarantee one seat.
  EXPECT_EQ(CumulativeVotingSharesNeeded(1000, 5, 1), 167);
  EXPECT_EQ(CumulativeVotingSharesNeeded(1000, 5, 3), 501);
  EXPECT_EQ(CumulativeVotingSharesNeeded(1000, 5, 0), 1);
  EXPECT_EQ(MajorityVotingSharesNeeded(1000), 501);
  // A minority of 167 cannot be shut out: the other 833 shares x 5 votes
  // spread over 5 candidates give each 833 < 167 x 5 = 835.
  EXPECT_LT(833 * 5 / 5, 167 * 5);
}

TEST(SplitTest, AdjustedPricesGiveCleanReturns) {
  EXPECT_DOUBLE_EQ(PriceAfterSplit(90.0, 3.0), 30.0);
  EXPECT_DOUBLE_EQ(PriceAfterSplit(55.0, SplitRatioFromStockDividend(0.10)),
                   50.0);
  // A 2-for-1 split on day 2.
  const std::vector<double> prices{100.0, 104.0, 52.5, 53.0};
  const std::vector<double> ratios{1.0, 1.0, 2.0, 1.0};
  const auto adj = SplitAdjustPrices(prices, ratios);
  EXPECT_DOUBLE_EQ(adj[0], 50.0);
  EXPECT_DOUBLE_EQ(adj[1], 52.0);
  EXPECT_DOUBLE_EQ(adj[2], 52.5);
  EXPECT_DOUBLE_EQ(adj[3], 53.0);
}

TEST(RightsTest, ValueOfARight) {
  const RightsTerms terms{4, 40.0};
  const double cum = 50.0;
  const double ex = ExRightsPrice(cum, terms);
  EXPECT_DOUBLE_EQ(ex, 48.0);
  EXPECT_DOUBLE_EQ(RightValueCum(cum, terms), 2.0);
  EXPECT_DOUBLE_EQ(RightValueEx(ex, terms), 2.0);
  // Holder wealth is unchanged: cum price = ex price + one right.
  EXPECT_DOUBLE_EQ(cum, ex + RightValueEx(ex, terms));
}

}  // namespace
}  // namespace investm::ch16
