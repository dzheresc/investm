#include "cpp/ch11_apt/apt.h"

#include <cmath>
#include <numeric>

#include "gtest/gtest.h"

namespace investm::ch11 {
namespace {

// One-factor example: four securities.
const Matrix kB({{0.9}, {1.1}, {1.5}, {0.5}});

TEST(AptTest, ArbitrageExistsWhenReturnsAreOffThePlane) {
  const Vector mu{0.10, 0.12, 0.20, 0.07};  // 0.20 is too high for b = 1.5.
  const auto x = FindArbitragePortfolio(mu, kB, 1.0);
  ASSERT_TRUE(x.has_value());
  EXPECT_TRUE(IsArbitragePortfolio(*x, kB, mu));
  EXPECT_GT((*x)[2], 0.0);  // Buy the underpriced security.
}

TEST(AptTest, NoArbitrageOnThePlane) {
  Vector mu;
  for (std::size_t i = 0; i < 4; ++i) mu.push_back(0.04 + 0.06 * kB(i, 0));
  EXPECT_FALSE(FindArbitragePortfolio(mu, kB).has_value());
  const auto p = FitAptPricing(mu, kB);
  ASSERT_TRUE(p.has_value());
  EXPECT_NEAR(p->lambda0, 0.04, 1e-12);
  EXPECT_NEAR(p->lambdas[0], 0.06, 1e-12);
  for (double e : p->pricing_errors) EXPECT_NEAR(e, 0.0, 1e-12);
}

TEST(AptTest, PureFactorPortfolioEarnsLambdaZeroPlusPremium) {
  const Matrix b({{1.0, 0.2}, {0.5, 1.0}, {1.2, -0.3}, {0.8, 0.7}, {0.3, 0.1}});
  Vector mu;
  for (std::size_t i = 0; i < 5; ++i) {
    mu.push_back(AptExpectedReturn(0.03, Vector{0.05, 0.02}, b.Row(i)));
  }
  const auto w = PureFactorPortfolio(b, 1);
  ASSERT_TRUE(w.has_value());
  EXPECT_NEAR(std::accumulate(w->begin(), w->end(), 0.0), 1.0, 1e-12);
  const Vector sens = b.Transpose() * *w;
  EXPECT_NEAR(sens[0], 0.0, 1e-12);
  EXPECT_NEAR(sens[1], 1.0, 1e-12);
  EXPECT_NEAR(Dot(*w, mu), 0.03 + 0.02, 1e-12);
}

TEST(AptTest, RejectsNonArbitragePortfolios) {
  const Vector mu{0.10, 0.12, 0.20, 0.07};
  // Costs money: not an arbitrage portfolio.
  EXPECT_FALSE(IsArbitragePortfolio(Vector{0.1, 0.0, 0.0, 0.0}, kB, mu));
}

}  // namespace
}  // namespace investm::ch11
