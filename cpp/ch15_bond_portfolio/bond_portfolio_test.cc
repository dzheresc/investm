#include "cpp/ch15_bond_portfolio/bond_portfolio.h"

#include <cmath>
#include <vector>

#include "cpp/ch14_bond_analysis/duration.h"
#include "gtest/gtest.h"

namespace investm::ch15 {
namespace {

using ch05::CouponBond;
using ch05::PresentValueAtYield;

TEST(HorizonTest, UnchangedYieldEarnsTheYield) {
  // Hold to maturity, reinvest at the purchase yield: realized compound
  // yield equals yield-to-maturity.
  const auto r = HorizonAnalysis(100.0, 0.05, 6.0, 0.06, 6.0, 0.0, 0.06);
  EXPECT_NEAR(r.realized_compound_yield, 0.06, 1e-12);
  EXPECT_NEAR(r.coupon_income, 30.0, 1e-12);
  // Sell early with the yield unchanged: same answer.
  const auto s = HorizonAnalysis(100.0, 0.05, 6.0, 0.06, 2.0, 0.06, 0.06);
  EXPECT_NEAR(s.realized_compound_yield, 0.06, 1e-12);
  // Yield falls: capital gain, higher realized return at short horizons.
  const auto g = HorizonAnalysis(100.0, 0.05, 6.0, 0.06, 2.0, 0.04, 0.06);
  EXPECT_GT(g.realized_compound_yield, 0.06);
  EXPECT_NEAR(1.0 + g.total_return,
              (g.sale_price + g.coupon_income + g.interest_on_interest) /
                  g.purchase_price,
              1e-14);
}

TEST(ImmunizationTest, TwoBondsMatchValueAndDuration) {
  const auto h = ImmunizeTwoBonds(1000.0, 6.0, 2.0, 10.0);
  ASSERT_TRUE(h.has_value());
  EXPECT_NEAR(h->value1 + h->value2, 1000.0, 1e-12);
  const std::vector<double> v{h->value1, h->value2};
  const std::vector<double> d{2.0, 10.0};
  EXPECT_NEAR(PortfolioDuration(v, d), 6.0, 1e-12);
  EXPECT_FALSE(ImmunizeTwoBonds(1000.0, 12.0, 2.0, 10.0).has_value());
}

TEST(ImmunizationTest, SurplusIsMinimizedAtTheInitialYield) {
  // Liability: 1000 due in 7 years. Assets: a 3-year zero and a 12-year
  // zero, weighted so value and duration match at 5%.
  const double y = 0.05;
  const CashFlows liability{{7.0, 1000.0}};
  const double pv = PresentValueAtYield(liability, y);
  const auto h = ImmunizeTwoBonds(pv, 7.0, 3.0, 12.0).value();
  const std::vector<CashFlows> bonds{{{3.0, 1.0}}, {{12.0, 1.0}}};
  const std::vector<double> units{h.value1 * std::pow(1.0 + y, 3.0),
                                  h.value2 * std::pow(1.0 + y, 12.0)};
  const CashFlows assets = Aggregate(bonds, units);
  EXPECT_NEAR(Surplus(assets, liability, y), 0.0, 1e-9);
  for (double dy : {-0.02, -0.005, 0.005, 0.02}) {
    EXPECT_GT(Surplus(assets, liability, y + dy), 0.0);
  }
}

TEST(CashMatchTest, CoversEveryLiabilityExactly) {
  const std::vector<CashFlows> bonds{CouponBond(100.0, 0.04, 1.0),
                                     CouponBond(100.0, 0.05, 2.0),
                                     CouponBond(100.0, 0.06, 3.0)};
  const CashFlows liabilities{{1.0, 500.0}, {2.0, 800.0}, {3.0, 600.0}};
  const auto units = CashMatch(bonds, liabilities);
  ASSERT_TRUE(units.has_value());
  const CashFlows assets = Aggregate(bonds, *units);
  ASSERT_EQ(assets.size(), 3u);
  for (std::size_t i = 0; i < 3; ++i) {
    EXPECT_NEAR(assets[i].amount, liabilities[i].amount, 1e-9);
  }
  // No bond maturing in year 4 -> impossible.
  EXPECT_FALSE(CashMatch(bonds, CashFlows{{4.0, 100.0}}).has_value());
}

TEST(ContingentTest, Cushion) {
  const double need = RequiredImmunizedAssets(1000.0, 0.06, 5.0);
  EXPECT_NEAR(need * std::pow(1.03, 10), 1000.0, 1e-9);
  EXPECT_NEAR(ContingentCushion(800.0, 1000.0, 0.06, 5.0), 800.0 - need, 1e-12);
}

TEST(SwapTest, ParSwapHasZeroValue) {
  const std::vector<double> df{0.97, 0.94, 0.905, 0.87};
  const double par = ParSwapRate(df, 1.0);
  EXPECT_NEAR(PayFixedSwapValue(1e6, par, df, 1.0), 0.0, 1e-6);
  EXPECT_GT(PayFixedSwapValue(1e6, par - 0.005, df, 1.0), 0.0);
  // A par bond's coupon equals the par swap rate on the same curve.
  double pv = 0.0;
  for (double d : df) pv += par * d;
  pv += df.back();
  EXPECT_NEAR(pv, 1.0, 1e-14);
}

}  // namespace
}  // namespace investm::ch15
