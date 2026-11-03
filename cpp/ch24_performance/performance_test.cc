#include "cpp/ch24_performance/performance.h"

#include <algorithm>
#include <cmath>
#include <numeric>
#include <vector>

#include "gtest/gtest.h"

namespace investm::ch24 {
namespace {

TEST(ReturnsTest, TimeVsDollarWeighted) {
  // $100 grows 20% in year 1; the investor adds $100; then it falls 10%.
  const std::vector<double> value_end{120.0, 198.0};
  const std::vector<double> flows{100.0, 0.0};
  const auto r = PeriodReturnsFromValues(100.0, value_end, flows);
  EXPECT_NEAR(r[0], 0.20, 1e-15);
  EXPECT_NEAR(r[1], -0.10, 1e-15);
  EXPECT_NEAR(TimeWeightedReturn(r), 1.2 * 0.9 - 1.0, 1e-15);
  // Dollar-weighted: 100 (1+y)^2 + 100 (1+y) = 198.
  const auto dw =
      DollarWeightedReturn(100.0, std::vector<double>{100.0}, 198.0);
  ASSERT_TRUE(dw.has_value());
  EXPECT_NEAR(100 * (1 + *dw) * (1 + *dw) + 100 * (1 + *dw), 198.0, 1e-9);
  EXPECT_LT(*dw, 0.0);  // Bad timing of the deposit.
}

std::vector<double> Market() {
  return {0.04, -0.03, 0.06, 0.01,  -0.05, 0.07,
          0.02, -0.01, 0.05, -0.02, 0.03,  0.00};
}

TEST(EvaluateTest, KnownAlphaAndBeta) {
  const auto m = Market();
  const std::vector<double> rf(m.size(), 0.002);
  const std::vector<double> noise{0.001,   -0.002,  0.0015, -0.001,
                                  0.0005,  -0.0015, 0.002,  0.0,
                                  -0.0005, 0.001,   -0.001, 0.0005};
  std::vector<double> p;
  for (std::size_t t = 0; t < m.size(); ++t) {
    p.push_back(rf[t] + 0.003 + 1.2 * (m[t] - rf[t]) + noise[t]);
  }
  const auto s = Evaluate(p, m, rf);
  EXPECT_NEAR(s.beta, 1.2, 0.02);
  EXPECT_NEAR(s.alpha, 0.003, 0.0005);
  EXPECT_GT(s.alpha_t_stat, 3.0);
  EXPECT_NEAR(s.treynor * s.beta, s.mean_excess, 1e-15);
  EXPECT_GT(s.m_squared, 0.0);
  // The market itself has zero alpha, beta one and M^2 zero.
  const auto ms = Evaluate(m, m, rf);
  EXPECT_NEAR(ms.alpha, 0.0, 1e-15);
  EXPECT_NEAR(ms.beta, 1.0, 1e-12);
  EXPECT_NEAR(ms.m_squared, 0.0, 1e-15);
}

TEST(EvaluateTest, InformationRatio) {
  const std::vector<double> p{0.02, 0.01, 0.03};
  const std::vector<double> b{0.01, 0.01, 0.01};
  EXPECT_NEAR(InformationRatio(p, b), 0.01 / 0.01, 1e-12);
}

TEST(TimingTest, DetectsConvexity) {
  const auto m = Market();
  const std::vector<double> rf(m.size(), 0.0);
  std::vector<double> timer;
  std::vector<double> option_like;
  for (double x : m) {
    timer.push_back(0.8 * x + 3.0 * x * x);
    option_like.push_back(0.5 * x + 0.7 * std::max(0.0, -x));
  }
  const auto tm = TreynorMazuy(timer, m, rf);
  ASSERT_TRUE(tm.has_value());
  EXPECT_NEAR(tm->timing, 3.0, 1e-8);
  EXPECT_NEAR(tm->beta, 0.8, 1e-8);
  const auto hm = HenrikssonMerton(option_like, m, rf);
  ASSERT_TRUE(hm.has_value());
  EXPECT_NEAR(hm->timing, 0.7, 1e-8);
}

TEST(AttributionTest, ComponentsAddUp) {
  const std::vector<double> wp{0.70, 0.20, 0.10};
  const std::vector<double> wb{0.60, 0.30, 0.10};
  const std::vector<double> rp{0.055, 0.028, 0.011};
  const std::vector<double> rb{0.050, 0.025, 0.010};
  const auto a = BrinsonAttribution(wp, wb, rp, rb);
  const double sum =
      std::accumulate(a.allocation.begin(), a.allocation.end(), 0.0) +
      std::accumulate(a.selection.begin(), a.selection.end(), 0.0) +
      std::accumulate(a.interaction.begin(), a.interaction.end(), 0.0);
  EXPECT_NEAR(sum, a.Total(), 1e-15);
  EXPECT_GT(a.allocation[0], 0.0);  // Overweight the best segment.
}

TEST(StyleTest, RecoversMixture) {
  const auto m = Market();
  Matrix styles(m.size(), 3);
  std::vector<double> fund;
  for (std::size_t t = 0; t < m.size(); ++t) {
    styles(t, 0) = 0.002;  // Bills.
    styles(t, 1) = 0.3 * m[t] + 0.003 * std::cos(static_cast<double>(t));
    styles(t, 2) = 1.3 * m[t] + 0.004 * std::sin(static_cast<double>(t));
    fund.push_back(0.2 * styles(t, 0) + 0.5 * styles(t, 1) +
                   0.3 * styles(t, 2));
  }
  const auto fit = StyleAnalysis(fund, styles);
  EXPECT_NEAR(fit.weights[1], 0.5, 1e-6);
  EXPECT_NEAR(fit.weights[2], 0.3, 1e-6);
  EXPECT_NEAR(std::accumulate(fit.weights.begin(), fit.weights.end(), 0.0), 1.0,
              1e-12);
  EXPECT_NEAR(fit.r_squared, 1.0, 1e-9);
  for (double w : fit.weights) EXPECT_GE(w, 0.0);
}

}  // namespace
}  // namespace investm::ch24
