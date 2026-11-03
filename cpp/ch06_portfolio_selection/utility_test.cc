#include "cpp/ch06_portfolio_selection/utility.h"

#include <cmath>
#include <vector>

#include "gtest/gtest.h"

namespace investm::ch06 {
namespace {

// A fair coin: wealth 50 or 150.
const std::vector<double> kWealth{50.0, 150.0};
const std::vector<double> kProbs{0.5, 0.5};

TEST(UtilityTest, LogCertaintyEquivalentIsGeometricMean) {
  const Utility u(UtilityKind::kLog);
  EXPECT_NEAR(CertaintyEquivalent(u, kWealth, kProbs), std::sqrt(50.0 * 150.0),
              1e-10);
  EXPECT_GT(RiskPremium(u, kWealth, kProbs), 0.0);
}

TEST(UtilityTest, RiskAttitudeSignsTheRiskPremium) {
  EXPECT_NEAR(RiskPremium(Utility(UtilityKind::kLinear), kWealth, kProbs), 0.0,
              1e-12);
  EXPECT_LT(RiskPremium(Utility(UtilityKind::kConvex), kWealth, kProbs), 0.0);
  EXPECT_GT(RiskPremium(Utility(UtilityKind::kPower, 3.0), kWealth, kProbs),
            RiskPremium(Utility(UtilityKind::kLog), kWealth, kProbs));
}

TEST(UtilityTest, InverseUndoesUtility) {
  for (const Utility& u :
       {Utility(UtilityKind::kLog), Utility(UtilityKind::kPower, 2.5),
        Utility(UtilityKind::kExponential, 0.01),
        Utility(UtilityKind::kQuadratic, 0.002), Utility(UtilityKind::kLinear),
        Utility(UtilityKind::kConvex)}) {
    EXPECT_NEAR(u.Inverse(u(120.0)), 120.0, 1e-9);
  }
}

TEST(UtilityTest, MarginalUtilityMatchesFiniteDifference) {
  const Utility u(UtilityKind::kPower, 2.0);
  const double h = 1e-5;
  EXPECT_NEAR(u.Marginal(80.0), (u(80.0 + h) - u(80.0 - h)) / (2 * h), 1e-8);
  EXPECT_NEAR(Utility(UtilityKind::kExponential, 0.3).AbsoluteRiskAversion(5),
              0.3, 1e-15);
}

TEST(UtilityTest, IndifferenceCurveHasConstantUtility) {
  const std::vector<double> sigmas{0.0, 0.1, 0.2, 0.3};
  const auto e = IndifferenceCurve(0.05, sigmas, 0.5);
  for (std::size_t i = 0; i < sigmas.size(); ++i) {
    EXPECT_NEAR(MeanVarianceUtility(e[i], sigmas[i], 0.5), 0.05, 1e-15);
  }
  EXPECT_GT(e[3] - e[2], e[1] - e[0]);  // Convex: risk aversion.
}

}  // namespace
}  // namespace investm::ch06
