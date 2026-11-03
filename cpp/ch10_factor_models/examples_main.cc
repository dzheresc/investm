// Worked examples for Chapter 10. All data are simulated.

#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

#include "cpp/ch10_factor_models/factor_models.h"
#include "cpp/core/table.h"

namespace {

using investm::Matrix;
using investm::PrintHeading;
using investm::PrintPercent;
using investm::Vector;
namespace ch10 = investm::ch10;

// Two macro factors: unexpected growth in industrial production (GDP) and
// unexpected inflation (INF). Six stocks from two sectors.
ch10::FactorModel Truth() {
  return {Vector{0.004, 0.005, 0.003, 0.006, 0.004, 0.005},
          Matrix({{1.2, -0.4},
                  {1.0, -0.3},
                  {1.4, -0.6},
                  {0.4, 0.5},
                  {0.5, 0.3},
                  {0.3, 0.6}}),
          Vector{0.0004, 0.0005, 0.0006, 0.0003, 0.0004, 0.0005}};
}

struct Data {
  Matrix returns;
  Matrix factors;
};

Data Simulate(std::size_t months) {
  const ch10::FactorModel m = Truth();
  std::mt19937_64 rng(2024);
  std::normal_distribution<double> gdp(0.0, 0.03);
  std::normal_distribution<double> inf(0.0, 0.02);
  std::normal_distribution<double> z(0.0, 1.0);
  Data d{Matrix(months, 6), Matrix(months, 2)};
  for (std::size_t t = 0; t < months; ++t) {
    d.factors(t, 0) = gdp(rng);
    d.factors(t, 1) = inf(rng);
    for (std::size_t i = 0; i < 6; ++i) {
      d.returns(t, i) = m.intercepts[i] +
                        m.sensitivities(i, 0) * d.factors(t, 0) +
                        m.sensitivities(i, 1) * d.factors(t, 1) +
                        std::sqrt(m.residual_variances[i]) * z(rng);
    }
  }
  return d;
}

void ImpliedMoments() {
  PrintHeading("10.2-10.3 Moments implied by a two-factor model");
  const ch10::FactorModel m = Truth();
  const Matrix fcov({{0.0009, 0.0}, {0.0, 0.0004}});
  const Vector w{0.25, 0.25, 0.0, 0.25, 0.25, 0.0};
  const auto d = ch10::DecomposePortfolioVariance(m, fcov, w);
  const Vector b = ch10::PortfolioSensitivities(m, w);
  std::printf("  portfolio sensitivities: GDP %.3f, INF %.3f\n", b[0], b[1]);
  PrintPercent("factor share of portfolio variance",
               d.factor_variance / (d.factor_variance + d.residual_variance));
  constexpr int kN = 500;
  constexpr int kK = 2;
  std::printf(
      "  For %d stocks the full covariance matrix needs %d estimates; the\n"
      "  %d-factor model needs only %d.\n",
      kN, kN * (kN + 1) / 2, kK, kN * kK + kN + kK * (kK + 1) / 2);
}

void TimeSeries(const Data& d) {
  PrintHeading("10.4.1 Time-series estimation (240 months)");
  const auto fit = ch10::EstimateTimeSeries(d.returns, d.factors).value();
  const ch10::FactorModel truth = Truth();
  investm::Table table({"stock", "b_GDP", "true", "b_INF", "true", "sd(e) %"});
  for (std::size_t i = 0; i < 6; ++i) {
    table.AddRow(
        {static_cast<double>(i + 1), fit.sensitivities(i, 0),
         truth.sensitivities(i, 0), fit.sensitivities(i, 1),
         truth.sensitivities(i, 1), 100 * std::sqrt(fit.residual_variances[i])},
        3);
  }
  table.Print();
}

void CrossSectional(const Data& d) {
  PrintHeading("10.4.2 Cross-sectional estimation with sector dummies");
  const std::vector<int> sector{0, 0, 0, 1, 1, 1};
  const Matrix dummies = ch10::SectorDummies(sector, 2);
  const Matrix values =
      ch10::EstimateCrossSectional(d.returns, dummies, false).value();
  std::printf("  first 5 months of sector factor returns (%%):\n");
  for (std::size_t t = 0; t < 5; ++t) {
    std::printf("    month %zu: cyclical %+6.2f  defensive %+6.2f\n", t + 1,
                100 * values(t, 0), 100 * values(t, 1));
  }
}

void FactorAnalytic(const Data& d) {
  PrintHeading("10.4.3 Factor-analytic (principal components) estimation");
  const auto pf = ch10::EstimatePrincipalComponents(d.returns, 2);
  std::printf("  eigenvalues:");
  for (double v : pf.eigenvalues) std::printf(" %.5f", v);
  std::printf("\n");
  PrintPercent("variance explained by 2 statistical factors",
               pf.explained_fraction);
  std::printf(
      "  The statistical factors span roughly the same space as GDP\n"
      "  and INF, but they are unlabeled rotations of them.\n");
}

}  // namespace

int main() {
  ImpliedMoments();
  const Data d = Simulate(240);
  TimeSeries(d);
  CrossSectional(d);
  FactorAnalytic(d);
  return 0;
}
