#include "cpp/core/root_find.h"

#include <cmath>

#include "gtest/gtest.h"

namespace investm {
namespace {

TEST(RootFindTest, BrentAndBisectFindSqrtTwo) {
  auto f = [](double x) { return x * x - 2.0; };
  EXPECT_NEAR(*Brent(f, 0.0, 2.0), std::sqrt(2.0), 1e-12);
  EXPECT_NEAR(*Bisect(f, 0.0, 2.0), std::sqrt(2.0), 1e-11);
  EXPECT_NEAR(*BrentExpandUp(f, 0.0, 0.1), std::sqrt(2.0), 1e-12);
}

TEST(RootFindTest, UnbracketedRootReturnsNullopt) {
  auto f = [](double x) { return x * x + 1.0; };
  EXPECT_FALSE(Brent(f, -1.0, 1.0).has_value());
  EXPECT_FALSE(Bisect(f, -1.0, 1.0).has_value());
}

TEST(RootFindTest, NewtonConvergesOnCubic) {
  auto f = [](double x) { return x * x * x - x - 1.0; };
  auto df = [](double x) { return 3.0 * x * x - 1.0; };
  EXPECT_NEAR(*Newton(f, df, 1.5), 1.324717957244746, 1e-12);
}

TEST(RootFindTest, NewtonReportsZeroDerivative) {
  auto f = [](double x) { return x * x + 1.0; };
  auto df = [](double x) { return 2.0 * x; };
  EXPECT_FALSE(Newton(f, df, 0.0).has_value());
}

}  // namespace
}  // namespace investm
