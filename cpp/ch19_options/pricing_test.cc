#include "cpp/ch19_options/pricing.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
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

// Plain backward induction over the whole tree, the definition the O(n)
// European pricer must reproduce.
double TreeByBackwardInduction(const OptionSpec& spec, int steps) {
  const double dt = spec.years / steps;
  const double u = std::exp(spec.volatility * std::sqrt(dt));
  const double d = 1.0 / u;
  const double p =
      (std::exp((spec.rate - spec.dividend_yield) * dt) - d) / (u - d);
  const double disc = std::exp(-spec.rate * dt);
  std::vector<double> v(static_cast<std::size_t>(steps) + 1);
  for (int j = 0; j <= steps; ++j) {
    const double s = spec.spot * std::pow(u, 2 * j - steps);
    v[static_cast<std::size_t>(j)] = spec.type == OptionType::kCall
                                         ? std::max(s - spec.strike, 0.0)
                                         : std::max(spec.strike - s, 0.0);
  }
  for (int i = steps; i-- > 0;) {
    for (int j = 0; j <= i; ++j) {
      const auto k = static_cast<std::size_t>(j);
      v[k] = disc * (p * v[k + 1] + (1.0 - p) * v[k]);
    }
  }
  return v[0];
}

TEST(BinomialTest, EuropeanMatchesBackwardInduction) {
  for (OptionType type : {OptionType::kCall, OptionType::kPut}) {
    for (const double strike : {60.0, 95.0, 100.0, 140.0}) {
      for (const int steps : {1, 2, 7, 50, 501, 1200}) {
        OptionSpec s = Spec(type);
        s.strike = strike;
        s.dividend_yield = 0.015;
        const double expected = TreeByBackwardInduction(s, steps);
        EXPECT_NEAR(BinomialPrice(s, steps, false), expected,
                    1e-11 * (1.0 + expected))
            << "strike " << strike << " steps " << steps;
      }
    }
  }
}

TEST(BinomialTest, EuropeanFarOutOfTheMoneyIsTinyButNotNegative) {
  OptionSpec s = Spec(OptionType::kCall);
  s.strike = 1000.0;
  const double v = BinomialPrice(s, 3000, false);
  EXPECT_GE(v, 0.0);
  EXPECT_LT(v, 1e-12);
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
