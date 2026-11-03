#include "investm/formulas/performance.h"

#include <gtest/gtest.h>

#include <cmath>

namespace investm::formulas {
namespace {

TEST(PerformanceTest, InvestmentCompany) {
  EXPECT_NEAR(NetAssetValue(1'000'000.0, 50'000.0, 100'000.0), 9.5, 1e-12);
  EXPECT_NEAR(InvestmentCompanyReturn(10.5, 10.0, 0.2, 0.3), 0.1, 1e-12);
}

TEST(PerformanceTest, RiskToleranceAndCertaintyEquivalent) {
  EXPECT_NEAR(ImpliedRiskTolerance(0.08, 0.12, 0.04, 0.2), 0.5, 1e-12);
  EXPECT_NEAR(CertaintyEquivalentReturn(0.10, 0.2, 0.5), 0.02, 1e-12);
}

TEST(PerformanceTest, AlphasAndCharacteristicLine) {
  EXPECT_NEAR(ExPostAlpha(0.12, 0.10), 0.02, 1e-12);
  const double alpha = CapmExPostAlpha(0.15, 0.04, 0.10, 1.2);
  EXPECT_NEAR(alpha, 0.038, 1e-12);
  EXPECT_NEAR(CharacteristicLine(alpha, 1.2, 0.10, 0.04) + 0.04, 0.15, 1e-12);
}

TEST(PerformanceTest, Ratios) {
  EXPECT_NEAR(TreynorRatio(0.12, 0.04, 0.8), 0.1, 1e-12);
  EXPECT_NEAR(SharpeRatio(0.12, 0.04, 0.2), 0.4, 1e-12);
  EXPECT_NEAR(MSquared(0.12, 0.04, 0.2, 0.15), 0.1, 1e-12);
}

TEST(PerformanceTest, CharacteristicCurves) {
  EXPECT_NEAR(QuadraticCharacteristicCurve(0.01, 1.0, 2.0, 0.14, 0.04), 0.13,
              1e-12);
  EXPECT_NEAR(DummyCharacteristicLines(0.01, 1.0, 0.5, 0.14, 0.04), 0.11,
              1e-12);
  EXPECT_NEAR(DummyCharacteristicLines(0.01, 1.0, 0.5, -0.06, 0.04), -0.04,
              1e-12);
}

TEST(PerformanceTest, ForeignInvestment) {
  EXPECT_NEAR(ForeignReturn(0.10, -0.05), 1.10 * 0.95 - 1.0, 1e-12);
  EXPECT_NEAR(ForeignStdDev(0.2, 0.1, 1.0), 0.3, 1e-12);
  EXPECT_NEAR(ForeignStdDev(0.2, 0.1, 0.0), std::sqrt(0.05), 1e-12);
}

}  // namespace
}  // namespace investm::formulas
