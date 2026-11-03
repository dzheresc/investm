#include "cpp/ch05_riskless_securities/cash_flows.h"

#include <cmath>

#include "gtest/gtest.h"

namespace investm::ch05 {
namespace {

TEST(CashFlowsTest, ParBondYieldsItsCoupon) {
  const CashFlows bond = CouponBond(1000, 0.07, 10, 2);
  ASSERT_EQ(bond.size(), 20u);
  EXPECT_DOUBLE_EQ(bond.back().amount, 1035.0);
  EXPECT_NEAR(PresentValueAtYield(bond, 0.07, 2), 1000.0, 1e-9);
  EXPECT_NEAR(*YieldToMaturity(bond, 1000.0, 2), 0.07, 1e-10);
}

TEST(CashFlowsTest, YieldRoundTrip) {
  const CashFlows bond = CouponBond(1000, 0.05, 7);
  EXPECT_NEAR(*YieldToMaturity(bond, PresentValueAtYield(bond, 0.0831)), 0.0831,
              1e-10);
  // A very cheap bond needs the bracket to expand well past 20%.
  EXPECT_NEAR(*YieldToMaturity(bond, PresentValueAtYield(bond, 0.75)), 0.75,
              1e-10);
}

TEST(CashFlowsTest, ZeroCouponClosedForm) {
  EXPECT_NEAR(*YieldToMaturity(ZeroCoupon(1000, 3), 800.0),
              std::pow(1.25, 1.0 / 3.0) - 1.0, 1e-10);
}

TEST(CashFlowsTest, ContinuousPresentValue) {
  EXPECT_NEAR(PresentValueContinuous(ZeroCoupon(100, 2), 0.05),
              100 * std::exp(-0.1), 1e-12);
}

TEST(CashFlowsTest, InternalRateOfReturnZeroesNpv) {
  const CashFlows project{{0, -1000}, {1, 500}, {2, 500}, {3, 200}};
  const auto r = InternalRateOfReturn(project);
  ASSERT_TRUE(r.has_value());
  EXPECT_NEAR(PresentValueAtYield(project, *r), 0.0, 1e-8);
}

TEST(CashFlowsTest, NoIrrWithoutSignChange) {
  const CashFlows gift{{0, 100}, {1, 100}};
  EXPECT_FALSE(InternalRateOfReturn(gift).has_value());
}

}  // namespace
}  // namespace investm::ch05
