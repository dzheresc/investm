#include "cpp/ch05_riskless_securities/term_structure.h"

#include <cmath>
#include <vector>

#include "gtest/gtest.h"

namespace investm::ch05 {
namespace {

TEST(TermStructureTest, BootstrapRecoversKnownCurve) {
  const SpotCurve truth({0.04, 0.045, 0.05, 0.053, 0.055});
  std::vector<BondQuote> quotes;
  for (int n = 1; n <= 5; ++n) {
    const double c = 0.03 + 0.005 * n;
    quotes.push_back({100, c, truth.PresentValue(CouponBond(100, c, n))});
  }
  const auto fitted = Bootstrap(quotes);
  ASSERT_TRUE(fitted.has_value());
  for (int n = 1; n <= 5; ++n) {
    EXPECT_NEAR(fitted->Spot(n), truth.Spot(n), 1e-12) << "maturity " << n;
  }
}

TEST(TermStructureTest, BootstrapRejectsArbitragePrices) {
  // A 2-year bond priced below the value of its first coupon alone.
  const std::vector<BondQuote> quotes{{100, 0.05, 95}, {100, 0.05, 3}};
  EXPECT_FALSE(Bootstrap(quotes).has_value());
}

TEST(TermStructureTest, ForwardsDiscountFactorsAndSpotsAgree) {
  const SpotCurve c({0.05, 0.06, 0.065});
  EXPECT_NEAR(c.OneYearForward(1), 0.05, 1e-15);
  EXPECT_NEAR(c.OneYearForward(2), 1.06 * 1.06 / 1.05 - 1.0, 1e-14);
  EXPECT_NEAR(c.Forward(1, 3), std::sqrt(std::pow(1.065, 3) / 1.05) - 1.0,
              1e-14);
  const SpotCurve rebuilt = SpotCurve::FromForwards(
      {c.OneYearForward(1), c.OneYearForward(2), c.OneYearForward(3)});
  for (int t = 1; t <= 3; ++t) EXPECT_NEAR(rebuilt.Spot(t), c.Spot(t), 1e-14);
}

TEST(TermStructureTest, ParYields) {
  const SpotCurve c({0.05, 0.06, 0.065});
  EXPECT_NEAR(c.ParYield(1), 0.05, 1e-14);
  EXPECT_NEAR(c.PresentValue(CouponBond(1, c.ParYield(3), 3)), 1.0, 1e-14);
}

TEST(TermStructureTest, UnbiasedExpectationsEqualizesHprs) {
  const SpotCurve c({0.04, 0.05, 0.055, 0.058});
  const SpotCurve next = RollForwardOneYear(c);
  for (int n = 1; n <= 4; ++n) {
    EXPECT_NEAR(OneYearHprOfZero(c, next, n), 0.04, 1e-12);
  }
}

TEST(TermStructureTest, LiquidityPremiumsRoundTrip) {
  const std::vector<double> expected{0.05, 0.05, 0.05};
  const std::vector<double> premiums{0.0, 0.003, 0.005};
  const SpotCurve c = LiquidityPreferenceCurve(expected, premiums);
  const auto back = ImpliedLiquidityPremiums(c, expected);
  for (std::size_t i = 0; i < 3; ++i) EXPECT_NEAR(back[i], premiums[i], 1e-14);
  EXPECT_GT(c.Spot(3), c.Spot(1));
}

TEST(TermStructureDeathTest, OutOfRangeMaturityAborts) {
  const SpotCurve c({0.05});
  EXPECT_DEATH(c.Spot(2), "maturity out of range");
}

}  // namespace
}  // namespace investm::ch05
