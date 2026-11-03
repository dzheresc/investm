#include "cpp/ch25_international/international.h"

#include <cmath>
#include <vector>

#include "gtest/gtest.h"

namespace investm::ch25 {
namespace {

TEST(CurrencyTest, DomesticReturns) {
  const double e = CurrencyReturn(1.10, 1.21);
  EXPECT_NEAR(e, 0.10, 1e-14);
  EXPECT_NEAR(DomesticReturn(0.08, e), 1.08 * 1.10 - 1.0, 1e-14);
  // A foreign gain can be wiped out by a falling currency.
  EXPECT_LT(DomesticReturn(0.08, -0.10), 0.0);
  const auto r = DomesticReturns(std::vector<double>{0.1, -0.05},
                                 std::vector<double>{-0.02, 0.04});
  EXPECT_NEAR(r[1], 0.95 * 1.04 - 1.0, 1e-15);
  EXPECT_NEAR(DomesticReturnVariance(0.2, 0.1, 0.0), 0.05, 1e-15);
}

TEST(ParityTest, CoveredInterestParityHasNoArbitrage) {
  const double s = 1.25;
  const double f = ParityForward(s, 0.05, 0.02, 1.0);
  // Investing abroad and hedging earns the domestic rate.
  const double hedged = (1.0 + 0.02) * f / s - 1.0;
  EXPECT_NEAR(hedged, 0.05, 1e-14);
  EXPECT_GT(ForwardPremium(s, f), 0.0);  // Higher domestic rate.
  // A perfectly forecast foreign return, fully hedged, ignores the spot.
  EXPECT_NEAR(HedgedReturn(0.07, 0.07, s, f, 0.9),
              HedgedReturn(0.07, 0.07, s, f, 1.6), 1e-14);
}

TEST(PppTest, RelativePpp) {
  EXPECT_NEAR(PppExpectedChange(0.05, 0.02), 1.05 / 1.02 - 1.0, 1e-15);
  // If the spot moves with PPP, the real exchange rate is unchanged.
  const double s0 = 0.8;
  const double s1 = s0 * (1.0 + PppExpectedChange(0.05, 0.02));
  EXPECT_NEAR(RealExchangeRate(s0, 100.0, 100.0),
              RealExchangeRate(s1, 102.0, 105.0), 1e-14);
}

TEST(DiversificationTest, MinimumRisk) {
  const double w = MinimumRiskForeignWeight(0.16, 0.20, 0.5);
  const double best = TwoMarketStdDev(w, 0.16, 0.20, 0.5);
  EXPECT_LT(best, 0.16);
  for (double x = 0.0; x <= 1.0; x += 0.05) {
    EXPECT_GE(TwoMarketStdDev(x, 0.16, 0.20, 0.5), best - 1e-15);
  }
  EXPECT_NEAR(TwoMarketStdDev(0.5, 0.2, 0.2, 1.0), 0.2, 1e-15);
}

}  // namespace
}  // namespace investm::ch25
