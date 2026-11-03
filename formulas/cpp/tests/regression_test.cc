#include "investm/formulas/regression.h"

#include <gtest/gtest.h>

#include <cmath>
#include <cstddef>
#include <vector>

namespace investm::formulas {
namespace {

const std::vector<double> kX = {0.02, -0.01, 0.03, 0.015, -0.02, 0.01, 0.025};
const std::vector<double> kY = {0.025, -0.02, 0.04, 0.01, -0.015, 0.02, 0.03};

// Least squares by deviations from the mean, as an independent reference.
struct Reference {
  double alpha, beta, se, se_beta, se_alpha, rho;
};

Reference Fit() {
  const double t = static_cast<double>(kX.size());
  double mx = 0, my = 0, sum_xx = 0;
  for (std::size_t i = 0; i < kX.size(); ++i) {
    mx += kX[i] / t;
    my += kY[i] / t;
    sum_xx += kX[i] * kX[i];
  }
  double sxx = 0, syy = 0, sxy = 0;
  for (std::size_t i = 0; i < kX.size(); ++i) {
    sxx += (kX[i] - mx) * (kX[i] - mx);
    syy += (kY[i] - my) * (kY[i] - my);
    sxy += (kX[i] - mx) * (kY[i] - my);
  }
  Reference r;
  r.beta = sxy / sxx;
  r.alpha = my - r.beta * mx;
  double sse = 0;
  for (std::size_t i = 0; i < kX.size(); ++i) {
    const double e = kY[i] - r.alpha - r.beta * kX[i];
    sse += e * e;
  }
  r.se = std::sqrt(sse / (t - 2));
  r.se_beta = r.se / std::sqrt(sxx);
  r.se_alpha = r.se * std::sqrt(sum_xx / (t * sxx));
  r.rho = sxy / std::sqrt(sxx * syy);
  return r;
}

TEST(RegressionTest, MatchesLeastSquaresReference) {
  const Reference ref = Fit();
  const double beta = RegressionBeta(kX, kY);
  const double alpha = RegressionAlpha(kX, kY, beta);
  EXPECT_NEAR(beta, ref.beta, 1e-10);
  EXPECT_NEAR(alpha, ref.alpha, 1e-12);
  const double se = RegressionResidualStdDev(kX, kY, alpha, beta);
  EXPECT_NEAR(se, ref.se, 1e-9);
  EXPECT_NEAR(RegressionBetaStdError(kX, se), ref.se_beta, 1e-8);
  EXPECT_NEAR(RegressionAlphaStdError(kX, se), ref.se_alpha, 1e-9);
  EXPECT_NEAR(CorrelationCoefficient(kX, kY), ref.rho, 1e-10);
}

TEST(RegressionTest, BadInputGivesNaN) {
  const std::vector<double> short_y = {1.0};
  EXPECT_TRUE(std::isnan(RegressionBeta(kX, short_y)));
}

}  // namespace
}  // namespace investm::formulas
