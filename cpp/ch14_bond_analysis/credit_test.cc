#include "cpp/ch14_bond_analysis/credit.h"

#include "gtest/gtest.h"

namespace investm::ch14 {
namespace {

TEST(CreditTest, NoDefaultMeansExpectedEqualsPromised) {
  const auto flows = ExpectedCashFlows(100.0, 0.07, 5, 0.0, 0.4);
  const auto promised = ch05::CouponBond(100.0, 0.07, 5.0);
  ASSERT_EQ(flows.size(), promised.size());
  for (std::size_t i = 0; i < flows.size(); ++i) {
    EXPECT_DOUBLE_EQ(flows[i].amount, promised[i].amount);
  }
}

TEST(CreditTest, OneYearExpectedFlow) {
  const auto flows = ExpectedCashFlows(100.0, 0.08, 1, 0.05, 0.40);
  EXPECT_NEAR(flows[0].amount, 0.95 * 108.0 + 0.05 * 40.0, 1e-12);
}

TEST(CreditTest, YieldDecomposition) {
  const auto d = DecomposeYield(100.0, 0.08, 5, 97.0, 0.02, 0.45, 0.04);
  ASSERT_TRUE(d.has_value());
  EXPECT_GT(d->promised_yield, 0.08);
  EXPECT_GT(d->default_premium, 0.0);
  EXPECT_NEAR(d->promised_yield - d->default_premium, d->expected_yield, 1e-15);
  EXPECT_NEAR(d->expected_yield - 0.04, d->risk_premium, 1e-15);
}

TEST(CreditTest, ImpliedDefaultProbability) {
  const double p = ImpliedDefaultProbability(0.07, 0.04, 0.40);
  EXPECT_NEAR(1.07 * (1.0 - p * 0.60), 1.04, 1e-14);
  EXPECT_NEAR(ImpliedDefaultProbability(0.04, 0.04, 0.4), 0.0, 1e-15);
}

TEST(CreditTest, DiscriminantSeparatesGroups) {
  // Two ratios: interest coverage and debt / assets.
  const Matrix healthy({{8.0, 0.30}, {6.5, 0.35}, {9.2, 0.25}, {7.1, 0.40}});
  const Matrix failed({{1.2, 0.75}, {2.0, 0.65}, {0.8, 0.80}, {1.5, 0.70}});
  const auto lda = LinearDiscriminant::Fit(healthy, failed);
  ASSERT_TRUE(lda.has_value());
  for (std::size_t r = 0; r < 4; ++r) {
    EXPECT_TRUE(lda->IsGroupA(healthy.Row(r)));
    EXPECT_FALSE(lda->IsGroupA(failed.Row(r)));
  }
  EXPECT_GT(lda->weights()[0], 0.0);  // Coverage helps.
  EXPECT_FALSE(lda->IsGroupA(Vector{1.0, 0.85}));
}

}  // namespace
}  // namespace investm::ch14
