#include "cpp/ch20_futures/futures.h"

#include <cmath>
#include <vector>

#include "cpp/ch19_options/pricing.h"
#include "gtest/gtest.h"

namespace investm::ch20 {
namespace {

TEST(MarginTest, MarkToMarketAndCalls) {
  // Long 2 contracts x 100 at 50; initial 1,000, maintenance 750.
  const std::vector<double> settle{49.0, 47.5, 48.0, 51.0};
  const auto days = MarkToMarket(50.0, settle, 2.0, 100.0, 1000.0, 750.0);
  ASSERT_EQ(days.size(), 4u);
  EXPECT_DOUBLE_EQ(days[0].gain, -200.0);
  EXPECT_DOUBLE_EQ(days[0].equity, 800.0);
  EXPECT_DOUBLE_EQ(days[1].gain, -300.0);
  EXPECT_DOUBLE_EQ(days[1].margin_call, 500.0);  // 500 -> back to 1,000.
  EXPECT_DOUBLE_EQ(days[1].equity, 1000.0);
  EXPECT_DOUBLE_EQ(days[3].equity, 1000.0 + 100.0 + 600.0);
  // Total gains equal the price change times the position size.
  double total = 0.0;
  for (const auto& d : days) total += d.gain;
  EXPECT_DOUBLE_EQ(total, 2.0 * 100.0 * (51.0 - 50.0));
}

TEST(CarryTest, PricesAndBand) {
  EXPECT_NEAR(CostOfCarryPrice(100.0, 0.05, 0.02, 0.0, 1.0),
              100.0 * std::exp(0.03), 1e-12);
  EXPECT_DOUBLE_EQ(CostOfCarryPrice(100.0, 0.05, 0.02, 0.0, 0.0), 100.0);
  const std::vector<Dividend> divs{{0.25, 1.0}, {0.75, 1.0}, {1.25, 1.0}};
  EXPECT_NEAR(IndexFuturesPrice(100.0, 0.04, divs, 1.0),
              (100.0 - std::exp(-0.01) - std::exp(-0.03)) * std::exp(0.04),
              1e-12);
  const auto band = NoArbitrageBand(100.0, 0.04, 0.06, 0.0, 0.002, 0.5);
  EXPECT_LT(band.lower, CostOfCarryPrice(100.0, 0.05, 0.0, 0.0, 0.5));
  EXPECT_GT(band.upper, CostOfCarryPrice(100.0, 0.05, 0.0, 0.0, 0.5));
  // Equal rates and no costs collapse the band to a point.
  const auto point = NoArbitrageBand(100.0, 0.05, 0.05, 0.01, 0.0, 0.5);
  EXPECT_NEAR(point.lower, point.upper, 1e-12);
  EXPECT_DOUBLE_EQ(Basis(100.0, 101.5), -1.5);
}

TEST(HedgeTest, MinimumVariance) {
  const std::vector<double> df{0.8, -1.1, 0.3, 1.5, -0.6, -0.9, 0.4, 1.2};
  std::vector<double> ds;
  const std::vector<double> noise{0.05, -0.02, 0.01,  -0.04,
                                  0.03, 0.00,  -0.01, 0.02};
  for (std::size_t t = 0; t < df.size(); ++t) {
    ds.push_back(0.9 * df[t] + noise[t]);
  }
  const auto fit = MinimumVarianceHedge(ds, df);
  EXPECT_NEAR(fit.beta, 0.9, 0.02);
  const double best = HedgeEffectiveness(ds, df, fit.beta);
  EXPECT_GT(best, 0.99);
  EXPECT_GT(best, HedgeEffectiveness(ds, df, 1.0));
  EXPECT_NEAR(best, fit.r_squared, 1e-12);
  EXPECT_DOUBLE_EQ(ContractsToHedge(0.9, 1e6, 5000.0), 180.0);
}

TEST(HedgeTest, BetaTargeting) {
  // $20m at beta 1.2 -> beta 0: sell 1.2 x 20m / (4000 x 50) = 120.
  EXPECT_DOUBLE_EQ(IndexFuturesContracts(2e7, 1.2, 0.0, 4000.0, 50.0), -120.0);
  EXPECT_DOUBLE_EQ(IndexFuturesContracts(2e7, 1.2, 1.5, 4000.0, 50.0), 30.0);
}

TEST(BlackTest, MatchesBlackScholesWithCarry) {
  // An option on futures F = S e^{rT} equals an option on the stock with a
  // dividend yield equal to r.
  const double s = 100.0;
  const double r = 0.05;
  const double t = 0.5;
  const double f = s * std::exp(r * t);
  const ch19::OptionSpec spec{ch19::OptionType::kCall, s, 98.0, r, 0.3, t, 0.0};
  EXPECT_NEAR(
      BlackFuturesOptionPrice(ch19::OptionType::kCall, f, 98.0, r, 0.3, t),
      ch19::BlackScholesPrice(spec), 1e-10);
  const double c =
      BlackFuturesOptionPrice(ch19::OptionType::kCall, f, 98.0, r, 0.3, t);
  const double p =
      BlackFuturesOptionPrice(ch19::OptionType::kPut, f, 98.0, r, 0.3, t);
  EXPECT_NEAR(c - p, std::exp(-r * t) * (f - 98.0), 1e-12);
}

}  // namespace
}  // namespace investm::ch20
