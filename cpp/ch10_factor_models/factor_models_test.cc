#include "cpp/ch10_factor_models/factor_models.h"

#include <cmath>
#include <random>
#include <vector>

#include "gtest/gtest.h"

namespace investm::ch10 {
namespace {

FactorModel TwoFactorModel() {
  return {Vector{0.01, 0.02, 0.015},
          Matrix({{1.0, 0.5}, {0.8, -0.2}, {1.5, 1.0}}),
          Vector{0.010, 0.020, 0.030}};
}

TEST(FactorModelTest, ImpliedMoments) {
  const FactorModel m = TwoFactorModel();
  const Vector mu = FactorExpectedReturns(m, Vector{0.05, 0.02});
  EXPECT_NEAR(mu[0], 0.01 + 0.05 + 0.01, 1e-15);
  const Matrix fcov({{0.04, 0.0}, {0.0, 0.01}});
  const Matrix cov = FactorCovariance(m, fcov);
  EXPECT_NEAR(cov(0, 1), 1.0 * 0.8 * 0.04 + 0.5 * -0.2 * 0.01, 1e-15);
  EXPECT_NEAR(cov(2, 2), 2.25 * 0.04 + 1.0 * 0.01 + 0.03, 1e-15);
}

TEST(FactorModelTest, PortfolioDecompositionMatchesCovariance) {
  const FactorModel m = TwoFactorModel();
  const Matrix fcov({{0.04, 0.005}, {0.005, 0.01}});
  const Vector w{0.3, 0.3, 0.4};
  const auto d = DecomposePortfolioVariance(m, fcov, w);
  EXPECT_NEAR(d.factor_variance + d.residual_variance,
              QuadraticForm(w, FactorCovariance(m, fcov)), 1e-15);
  const Vector b = PortfolioSensitivities(m, w);
  EXPECT_NEAR(b[0], 0.3 + 0.24 + 0.6, 1e-15);
}

// Simulates T periods of a two-factor economy.
struct Simulated {
  Matrix returns;
  Matrix factors;
};
Simulated Simulate(const FactorModel& m, std::size_t periods) {
  std::mt19937_64 rng(5);
  std::normal_distribution<double> z(0.0, 1.0);
  Simulated s{Matrix(periods, m.num_securities()),
              Matrix(periods, m.num_factors())};
  for (std::size_t t = 0; t < periods; ++t) {
    for (std::size_t k = 0; k < m.num_factors(); ++k) {
      s.factors(t, k) = 0.03 * z(rng);
    }
    for (std::size_t i = 0; i < m.num_securities(); ++i) {
      double r = m.intercepts[i];
      for (std::size_t k = 0; k < m.num_factors(); ++k) {
        r += m.sensitivities(i, k) * s.factors(t, k);
      }
      s.returns(t, i) = r + std::sqrt(m.residual_variances[i]) * 0.1 * z(rng);
    }
  }
  return s;
}

TEST(FactorModelTest, TimeSeriesEstimationRecoversSensitivities) {
  const FactorModel truth = TwoFactorModel();
  const Simulated s = Simulate(truth, 2000);
  const auto fit = EstimateTimeSeries(s.returns, s.factors);
  ASSERT_TRUE(fit.has_value());
  for (std::size_t i = 0; i < 3; ++i) {
    for (std::size_t k = 0; k < 2; ++k) {
      EXPECT_NEAR(fit->sensitivities(i, k), truth.sensitivities(i, k), 0.05);
    }
  }
}

TEST(FactorModelTest, CrossSectionalEstimationRecoversFactorValues) {
  // Exact model: r_i = 0.01 + attr_i * F_t, so each period's regression is
  // exact.
  const Matrix attrs({{0.5}, {1.0}, {1.5}, {2.0}});
  const std::vector<double> f{0.02, -0.01, 0.03};
  Matrix r(3, 4);
  for (std::size_t t = 0; t < 3; ++t) {
    for (std::size_t i = 0; i < 4; ++i) r(t, i) = 0.01 + attrs(i, 0) * f[t];
  }
  const auto values = EstimateCrossSectional(r, attrs);
  ASSERT_TRUE(values.has_value());
  for (std::size_t t = 0; t < 3; ++t) {
    EXPECT_NEAR((*values)(t, 0), 0.01, 1e-12);
    EXPECT_NEAR((*values)(t, 1), f[t], 1e-12);
  }
}

TEST(FactorModelTest, SectorDummiesGiveSectorAverages) {
  const std::vector<int> sector{0, 0, 1, 1, 1};
  const Matrix d = SectorDummies(sector, 2);
  Matrix r(1, 5);
  const double row[] = {0.01, 0.03, -0.02, 0.00, 0.02};
  for (std::size_t i = 0; i < 5; ++i) r(0, i) = row[i];
  const auto values = EstimateCrossSectional(r, d, false);
  ASSERT_TRUE(values.has_value());
  EXPECT_NEAR((*values)(0, 0), 0.02, 1e-14);
  EXPECT_NEAR((*values)(0, 1), 0.0, 1e-14);
}

TEST(FactorModelTest, PrincipalComponentsFindTheDominantFactor) {
  FactorModel one{Vector(6, 0.0), Matrix(6, 1), Vector(6, 0.0004)};
  for (std::size_t i = 0; i < 6; ++i) {
    one.sensitivities(i, 0) = 0.5 + 0.2 * static_cast<double>(i);
  }
  const Simulated s = Simulate(one, 3000);
  const PrincipalFactors pf = EstimatePrincipalComponents(s.returns, 1);
  EXPECT_GT(pf.explained_fraction, 0.95);
  // Loadings are proportional to the true sensitivities.
  const double ratio = pf.loadings(0, 0) / one.sensitivities(0, 0);
  for (std::size_t i = 1; i < 6; ++i) {
    EXPECT_NEAR(pf.loadings(i, 0) / one.sensitivities(i, 0), ratio,
                0.05 * ratio);
  }
}

}  // namespace
}  // namespace investm::ch10
