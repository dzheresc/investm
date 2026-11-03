#include "cpp/ch17_ddm/ddm.h"

#include <vector>

#include "gtest/gtest.h"

namespace investm::ch17 {
namespace {

TEST(DdmTest, ZeroAndConstantGrowth) {
  EXPECT_DOUBLE_EQ(ZeroGrowthValue(3.0, 0.12), 25.0);
  EXPECT_NEAR(ConstantGrowthValue(2.0, 0.05, 0.10), 42.0, 1e-12);
  EXPECT_NEAR(ConstantGrowthImpliedReturn(42.0, 2.0, 0.05), 0.10, 1e-15);
  EXPECT_NEAR(ConstantGrowthValue(2.0, 0.0, 0.10), ZeroGrowthValue(2.0, 0.10),
              1e-12);
}

TEST(DdmTest, MultipleGrowthReducesToConstantGrowth) {
  const double d0 = 1.5;
  const double g = 0.04;
  const double k = 0.09;
  std::vector<double> d;
  double x = d0;
  for (int t = 0; t < 6; ++t) d.push_back(x *= 1.0 + g);
  EXPECT_NEAR(MultipleGrowthValue(d, g, k), ConstantGrowthValue(d0, g, k),
              1e-10);
}

TEST(DdmTest, ImpliedReturnRoundTrip) {
  const std::vector<double> d{1.0, 1.25, 1.5, 1.7};
  const double v = MultipleGrowthValue(d, 0.05, 0.11);
  const auto k = MultipleGrowthImpliedReturn(v, d, 0.05);
  ASSERT_TRUE(k.has_value());
  EXPECT_NEAR(*k, 0.11, 1e-10);
}

TEST(DdmTest, ThreePhase) {
  const auto d = ThreePhaseDividends(1.0, 0.20, 2, 5, 0.05);
  ASSERT_EQ(d.size(), 5u);
  EXPECT_NEAR(d[0], 1.2, 1e-15);
  EXPECT_NEAR(d[1], 1.44, 1e-14);
  EXPECT_NEAR(d[2], 1.44 * 1.15, 1e-14);
  EXPECT_NEAR(d[3], 1.44 * 1.15 * 1.10, 1e-14);
  EXPECT_NEAR(d[4], 1.44 * 1.15 * 1.10 * 1.05, 1e-14);
  // Same growth in every phase -> constant growth.
  EXPECT_NEAR(ThreePhaseValue(1.0, 0.05, 2, 5, 0.05, 0.10),
              ConstantGrowthValue(1.0, 0.05, 0.10), 1e-10);
}

TEST(DdmTest, PriceEarningsAndGrowth) {
  EXPECT_NEAR(ForwardPriceEarnings(0.4, 0.06, 0.10), 10.0, 1e-12);
  EXPECT_NEAR(TrailingPriceEarnings(0.4, 0.06, 0.10), 10.6, 1e-12);
  EXPECT_NEAR(SustainableGrowth(0.15, 0.6), 0.09, 1e-15);
  // Retention adds value only when ROE > k.
  const double no_growth = ValueWithReinvestment(5.0, 0.0, 0.15, 0.12);
  EXPECT_NEAR(no_growth, 5.0 / 0.12, 1e-12);
  EXPECT_GT(ValueWithReinvestment(5.0, 0.5, 0.15, 0.12), no_growth);
  EXPECT_LT(ValueWithReinvestment(5.0, 0.5, 0.09, 0.12), no_growth);
  EXPECT_NEAR(ValueWithReinvestment(5.0, 0.5, 0.12, 0.12), no_growth, 1e-12);
  const double p = ValueWithReinvestment(5.0, 0.5, 0.15, 0.12);
  EXPECT_GT(PresentValueOfGrowthOpportunities(p, 5.0, 0.12), 0.0);
}

}  // namespace
}  // namespace investm::ch17
