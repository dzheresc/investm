#include "cpp/ch23_investment_management/management.h"

#include <cmath>
#include <vector>

#include "gtest/gtest.h"

namespace investm::ch23 {
namespace {

TEST(RiskToleranceTest, RoundTrip) {
  const double tau = ImpliedRiskTolerance(0.6, 0.09, 0.18, 0.03);
  EXPECT_NEAR(tau, 2 * 0.0324 * 0.6 / 0.06, 1e-15);
  EXPECT_NEAR(OptimalStockWeight(tau, 0.09, 0.18, 0.03), 0.6, 1e-15);
}

TEST(StrategyTest, BuyAndHoldIsLinearInTheRiskyReturn) {
  const std::vector<double> r{0.10, -0.20, 0.15};
  const auto p = BuyAndHold(r, 0.01, 0.6, 100.0);
  const double risky = 60.0 * 1.10 * 0.80 * 1.15;
  const double safe = 40.0 * std::pow(1.01, 3);
  EXPECT_NEAR(p.wealth.back(), risky + safe, 1e-10);
  EXPECT_EQ(p.wealth.size(), 4u);
  EXPECT_DOUBLE_EQ(p.turnover, 0.0);
}

TEST(StrategyTest, ConstantMixBeatsBuyAndHoldInReversals) {
  // Up, down, up, down with no net change: rebalancing buys low, sells high.
  const std::vector<double> r{0.25, -0.20, 0.25, -0.20};
  const auto mix = ConstantMix(r, 0.0, 0.5);
  const auto hold = BuyAndHold(r, 0.0, 0.5);
  EXPECT_NEAR(hold.wealth.back(), 1.0, 1e-12);
  EXPECT_GT(mix.wealth.back(), hold.wealth.back());
  // And loses in a trend.
  const std::vector<double> up{0.2, 0.2, 0.2, 0.2};
  EXPECT_LT(ConstantMix(up, 0.0, 0.5).wealth.back(),
            BuyAndHold(up, 0.0, 0.5).wealth.back());
  // A wide band means fewer trades.
  EXPECT_LT(ConstantMix(r, 0.0, 0.5, 0.2).turnover, mix.turnover);
}

TEST(StrategyTest, CppiProtectsTheFloor) {
  const std::vector<double> crash{-0.10, -0.15, -0.20, -0.25, -0.10};
  const auto p = Cppi(crash, 0.01, 3.0, 0.8);
  const double floor_end = 0.8 * std::pow(1.01, 5);
  EXPECT_GT(p.wealth.back(), floor_end - 1e-12);
  EXPECT_NEAR(p.risky_weight[0], 3.0 * 0.2, 1e-15);
  // Exposure falls as the cushion shrinks.
  for (std::size_t t = 1; t < p.risky_weight.size(); ++t) {
    EXPECT_LE(p.risky_weight[t], p.risky_weight[t - 1]);
  }
}

}  // namespace
}  // namespace investm::ch23
