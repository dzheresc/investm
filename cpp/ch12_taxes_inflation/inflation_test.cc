#include "cpp/ch12_taxes_inflation/inflation.h"

#include <vector>

#include "gtest/gtest.h"

namespace investm::ch12 {
namespace {

TEST(InflationTest, RatesFromIndex) {
  const std::vector<double> index{200.0, 210.0, 214.2, 220.626};
  const auto r = InflationRates(index);
  ASSERT_EQ(r.size(), 3u);
  EXPECT_NEAR(r[0], 0.05, 1e-15);
  EXPECT_NEAR(r[1], 0.02, 1e-14);
  EXPECT_NEAR(r[2], 0.03, 1e-14);
  EXPECT_NEAR(CumulativeInflation(index), 1.05 * 1.02 * 1.03 - 1.0, 1e-14);
  EXPECT_NEAR(PurchasingPower(index), 200.0 / 220.626, 1e-14);
}

TEST(InflationTest, RealReturnsAndFisher) {
  const std::vector<double> nominal{0.10, -0.02};
  const std::vector<double> infl{0.04, 0.01};
  const auto real = RealReturns(nominal, infl);
  EXPECT_NEAR(real[0], 1.10 / 1.04 - 1.0, 1e-15);
  EXPECT_NEAR(real[1], 0.98 / 1.01 - 1.0, 1e-15);
  EXPECT_NEAR(FisherNominalRate(real[0], 0.04), 0.10, 1e-15);
}

TEST(InflationTest, TaxesBiteHarderWithInflation) {
  // With no inflation the effective tax equals the statutory rate.
  EXPECT_NEAR(EffectiveTaxOnRealReturn(0.06, 0.30, 0.0), 0.30, 1e-14);
  const double low = EffectiveTaxOnRealReturn(0.06, 0.30, 0.01);
  const double high = EffectiveTaxOnRealReturn(0.09, 0.30, 0.04);
  EXPECT_GT(low, 0.30);
  EXPECT_GT(high, low);
  EXPECT_NEAR(RealAfterTaxReturn(0.10, 0.25, 0.05), 1.075 / 1.05 - 1.0, 1e-15);
}

TEST(InflationTest, IndexedBondAndBreakeven) {
  const std::vector<double> path{103.0, 106.09};
  const auto flows = IndexedBondCashFlows(1000.0, 0.02, 100.0, path);
  ASSERT_EQ(flows.size(), 2u);
  EXPECT_NEAR(flows[0].amount, 20.6, 1e-12);
  EXPECT_NEAR(flows[1].amount, 1060.9 * 1.02, 1e-9);
  // Deflation floor protects the principal.
  const std::vector<double> deflation{98.0};
  const auto d = IndexedBondCashFlows(1000.0, 0.02, 100.0, deflation);
  EXPECT_NEAR(d[0].amount, 19.6 + 1000.0, 1e-12);
  // If realized inflation equals break-even, nominal and real bonds tie.
  const double be = BreakevenInflation(0.05, 0.02);
  EXPECT_NEAR((1.02) * (1.0 + be), 1.05, 1e-15);
}

TEST(InflationTest, InflationBeta) {
  const std::vector<double> infl{0.01, 0.03, 0.02, 0.05, 0.04};
  std::vector<double> hedge;
  for (double i : infl) hedge.push_back(0.02 + i);
  const auto fit = InflationBeta(hedge, infl);
  EXPECT_NEAR(fit.beta, 1.0, 1e-12);
  EXPECT_NEAR(fit.alpha, 0.02, 1e-12);
}

}  // namespace
}  // namespace investm::ch12
