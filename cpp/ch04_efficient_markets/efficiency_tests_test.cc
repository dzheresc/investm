#include "cpp/ch04_efficient_markets/efficiency_tests.h"

#include <cmath>
#include <vector>

#include "cpp/core/stats.h"
#include "gtest/gtest.h"

namespace investm::ch04 {
namespace {

TEST(RunsTestTest, DetectsTooManyAndTooFewRuns) {
  const std::vector<double> alternating{1, -1, 1, -1, 1, -1,
                                        1, -1, 1, -1, 1, -1};
  EXPECT_GT(RunsTest(alternating).z, 2.0);
  const std::vector<double> clustered{1, 1, 1, 1, 1, 1, -1, -1, -1, -1, -1, -1};
  EXPECT_LT(RunsTest(clustered).z, -2.0);
  EXPECT_EQ(RunsTest(clustered).runs, 2u);
}

TEST(RandomWalkTest, PassesWeakFormTests) {
  const auto prices = RandomWalkPrices(100, 5000, 0.0, 0.01, 123);
  const auto r = LogReturns(prices);
  EXPECT_LT(std::fabs(stats::Autocorrelation(r, 1)), 0.05);
  EXPECT_LT(std::fabs(VarianceRatio(r, 5) - 1.0), 0.1);
  EXPECT_LT(std::fabs(RunsTest(r).z), 3.0);
}

TEST(VarianceRatioTest, MatchesDirectComputation) {
  const std::vector<double> r{0.01, -0.02, 0.015, 0.003, -0.007, 0.02, -0.01};
  std::vector<double> sums;
  for (std::size_t t = 0; t + 3 <= r.size(); ++t) {
    sums.push_back(r[t] + r[t + 1] + r[t + 2]);
  }
  EXPECT_NEAR(VarianceRatio(r, 3),
              stats::Variance(sums) / (3.0 * stats::Variance(r)), 1e-14);
}

TEST(FilterRuleTest, MonotoneRise) {
  const std::vector<double> prices{100, 102, 104, 106, 108, 110};
  // Trough is 100, so the rule buys at 102 and holds to the end.
  EXPECT_NEAR(FilterRuleReturn(prices, 0.02), 110.0 / 102.0 - 1.0, 1e-12);
}

TEST(EventStudyTest, DetectsInjectedAbnormalReturn) {
  std::vector<EventFirm> firms;
  for (int f = 0; f < 5; ++f) {
    EventFirm firm;
    for (int d = 0; d < 30; ++d) {
      const double m = 0.01 * std::sin(d + f);
      double r = 0.001 + 1.2 * m + 0.002 * std::cos(3.0 * d + f);
      if (d == 26) r += 0.05;
      firm.market.push_back(m);
      firm.stock.push_back(r);
    }
    firms.push_back(firm);
  }
  const EventStudyResult res = EventStudy(firms, 25);
  ASSERT_EQ(res.aar.size(), 5u);
  EXPECT_GT(res.aar[1], 0.04);
  EXPECT_LT(std::fabs(res.aar[0]), 0.01);
  EXPECT_GT(res.caar_t_stat, 3.0);
}

}  // namespace
}  // namespace investm::ch04
