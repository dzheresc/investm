#include "cpp/ch19_options/pricing.h"

#include <cmath>
#include <vector>

#include "gtest/gtest.h"

namespace investm::ch19 {
namespace {

OptionSpec Spec(OptionType type) {
  return {type, 100.0, 95.0, 0.04, 0.25, 0.75, 0.0};
}

TEST(BinomialTest, OnePeriodReplication) {
  // S = 50 goes to 60 or 40; r = 5%; K = 50.
  const Replication r =
      OnePeriodBinomial(OptionType::kCall, 50.0, 50.0, 1.2, 0.8, 0.05);
  EXPECT_NEAR(r.shares, 0.5, 1e-15);
  EXPECT_NEAR(r.bond, -40.0 * 0.5 / 1.05, 1e-12);
  // Replicating portfolio pays off like the option in both states.
  EXPECT_NEAR(r.shares * 60.0 + r.bond * 1.05, 10.0, 1e-12);
  EXPECT_NEAR(r.shares * 40.0 + r.bond * 1.05, 0.0, 1e-12);
  // Same answer via risk-neutral probability.
  const double p = (1.05 - 0.8) / (1.2 - 0.8);
  EXPECT_NEAR(r.value, p * 10.0 / 1.05, 1e-12);
}

TEST(BinomialTest, ConvergesToBlackScholes) {
  for (OptionType type : {OptionType::kCall, OptionType::kPut}) {
    const OptionSpec s = Spec(type);
    EXPECT_NEAR(BinomialPrice(s, 2000, false), BlackScholesPrice(s), 5e-3);
  }
}

TEST(BinomialTest, AmericanOptions) {
  // American call on a non-dividend stock is never exercised early.
  const OptionSpec call = Spec(OptionType::kCall);
  EXPECT_NEAR(BinomialPrice(call, 500, true), BinomialPrice(call, 500, false),
              1e-10);
  // An American put is worth more than a European one.
  OptionSpec put = Spec(OptionType::kPut);
  put.rate = 0.08;
  EXPECT_GT(BinomialPrice(put, 500, true), BinomialPrice(put, 500, false));
}

TEST(BlackScholesTest, ParityAndGreeks) {
  OptionSpec c = Spec(OptionType::kCall);
  c.dividend_yield = 0.02;
  OptionSpec p = c;
  p.type = OptionType::kPut;
  const double lhs = BlackScholesPrice(c) - BlackScholesPrice(p);
  const double rhs = c.spot * std::exp(-0.02 * c.years) -
                     c.strike * std::exp(-c.rate * c.years);
  EXPECT_NEAR(lhs, rhs, 1e-12);

  const Greeks g = BlackScholesGreeks(c);
  const double h = 1e-4;
  auto bump = [&](auto field, double dx) {
    OptionSpec s = c;
    s.*field += dx;
    return BlackScholesPrice(s);
  };
  EXPECT_NEAR(
      g.delta,
      (bump(&OptionSpec::spot, h) - bump(&OptionSpec::spot, -h)) / (2 * h),
      1e-6);
  EXPECT_NEAR(
      g.gamma,
      (bump(&OptionSpec::spot, h) - 2 * g.price + bump(&OptionSpec::spot, -h)) /
          (h * h),
      1e-4);
  EXPECT_NEAR(
      g.vega,
      (bump(&OptionSpec::volatility, h) - bump(&OptionSpec::volatility, -h)) /
          (2 * h),
      1e-5);
  EXPECT_NEAR(
      g.rho,
      (bump(&OptionSpec::rate, h) - bump(&OptionSpec::rate, -h)) / (2 * h),
      1e-5);
  EXPECT_NEAR(
      g.theta,
      -(bump(&OptionSpec::years, h) - bump(&OptionSpec::years, -h)) / (2 * h),
      1e-5);
}

TEST(VolatilityTest, ImpliedAndHistorical) {
  OptionSpec s = Spec(OptionType::kPut);
  const double price = BlackScholesPrice(s);
  s.volatility = 0.9;  // Starting value is ignored.
  const auto vol = ImpliedVolatility(s, price);
  ASSERT_TRUE(vol.has_value());
  EXPECT_NEAR(*vol, 0.25, 1e-9);
  OptionSpec itm = s;
  itm.strike = 120.0;  // Worth at least K e^{-rT} - S ~ 16.5.
  EXPECT_FALSE(ImpliedVolatility(itm, 10.0).has_value());
  EXPECT_FALSE(ImpliedVolatility(s, 1000.0).has_value());
  // Prices alternating +-1% log moves: sd of log relatives = 0.01 x n/(n-1).
  std::vector<double> prices{100.0};
  for (int t = 0; t < 10; ++t) {
    prices.push_back(prices.back() * std::exp(t % 2 == 0 ? 0.01 : -0.01));
  }
  EXPECT_NEAR(HistoricalVolatility(prices, 252.0),
              0.01 * std::sqrt(10.0 / 9.0) * std::sqrt(252.0), 1e-12);
}

TEST(WarrantTest, DilutionDiscount) {
  const OptionSpec s = Spec(OptionType::kCall);
  EXPECT_NEAR(WarrantValue(s, 1e6, 0.0), BlackScholesPrice(s), 1e-12);
  EXPECT_NEAR(WarrantValue(s, 1e6, 2.5e5), 0.8 * BlackScholesPrice(s), 1e-12);
}

}  // namespace
}  // namespace investm::ch19
