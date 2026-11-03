#include "investm/formulas/derivatives.h"

#include <gtest/gtest.h>

#include <cmath>

namespace investm::formulas {
namespace {

TEST(DerivativesTest, IntrinsicValues) {
  EXPECT_DOUBLE_EQ(CallIntrinsicValue(110.0, 100.0), 10.0);
  EXPECT_DOUBLE_EQ(CallIntrinsicValue(90.0, 100.0), 0.0);
  EXPECT_DOUBLE_EQ(PutIntrinsicValue(90.0, 100.0), 10.0);
  EXPECT_DOUBLE_EQ(PutIntrinsicValue(110.0, 100.0), 0.0);
}

TEST(DerivativesTest, BlackScholesReferenceValues) {
  EXPECT_NEAR(BlackScholesCall(100, 100, 0.05, 1.0, 0.2), 10.450584, 1e-5);
  EXPECT_NEAR(BlackScholesPut(100, 100, 0.05, 1.0, 0.2), 5.573526, 1e-5);
  EXPECT_NEAR(BlackScholesD1(100, 100, 0.05, 1.0, 0.2), 0.35, 1e-12);
  EXPECT_NEAR(BlackScholesD2(100, 100, 0.05, 1.0, 0.2), 0.15, 1e-12);
}

TEST(DerivativesTest, BlackScholesObeysPutCallParity) {
  const double s = 50.0, k = 55.0, r = 0.03, t = 0.5, v = 0.3;
  const double call = BlackScholesCall(s, k, r, t, v);
  const double put = BlackScholesPut(s, k, r, t, v);
  EXPECT_NEAR(PutFromParity(call, s, k, r, t), put, 1e-10);
  EXPECT_NEAR(CallFromParity(put, s, k, r, t), call, 1e-10);
}

TEST(DerivativesTest, Futures) {
  EXPECT_DOUBLE_EQ(FuturesFairValue(100.0, 5.0, 2.0, 1.0), 104.0);
  EXPECT_NEAR(IndexFuturesFairValue(1000.0, 0.05, 0.02), 1030.0, 1e-10);
}

}  // namespace
}  // namespace investm::formulas
