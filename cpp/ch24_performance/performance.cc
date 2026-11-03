#include "cpp/ch24_performance/performance.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "cpp/ch07_portfolio_analysis/efficient_set.h"
#include "cpp/core/check.h"
#include "cpp/core/regression.h"
#include "cpp/core/stats.h"

namespace investm::ch24 {
namespace {

std::vector<double> Excess(std::span<const double> r,
                           std::span<const double> rf) {
  INVESTM_CHECK(r.size() == rf.size(), "size mismatch");
  std::vector<double> e(r.size());
  for (std::size_t t = 0; t < r.size(); ++t) e[t] = r[t] - rf[t];
  return e;
}

std::optional<TimingFit> FitTiming(std::span<const double> portfolio,
                                   std::span<const double> market,
                                   std::span<const double> riskfree,
                                   bool quadratic) {
  const auto ep = Excess(portfolio, riskfree);
  const auto em = Excess(market, riskfree);
  INVESTM_CHECK(ep.size() == em.size() && ep.size() >= 5,
                "need at least five matching periods");
  Matrix x(em.size(), 2);
  for (std::size_t t = 0; t < em.size(); ++t) {
    x(t, 0) = em[t];
    x(t, 1) = quadratic ? em[t] * em[t] : std::max(0.0, -em[t]);
  }
  const auto fit = MultipleOls(x, ep);
  if (!fit) return std::nullopt;
  const double se = fit->std_errors[2];
  return TimingFit{fit->coefficients[0], fit->coefficients[1],
                   fit->coefficients[2],
                   se > 0.0 ? fit->coefficients[2] / se : 0.0};
}

}  // namespace

double TimeWeightedReturn(std::span<const double> period_returns) {
  double growth = 1.0;
  for (double r : period_returns) growth *= 1.0 + r;
  return growth - 1.0;
}

std::vector<double> PeriodReturnsFromValues(double start,
                                            std::span<const double> value_end,
                                            std::span<const double> flows) {
  INVESTM_CHECK(value_end.size() == flows.size(), "size mismatch");
  std::vector<double> r;
  r.reserve(value_end.size());
  double begin = start;
  for (std::size_t t = 0; t < value_end.size(); ++t) {
    INVESTM_CHECK(begin > 0.0, "portfolio value must stay positive");
    r.push_back(value_end[t] / begin - 1.0);
    begin = value_end[t] + flows[t];
  }
  return r;
}

std::optional<double> DollarWeightedReturn(double start,
                                           std::span<const double> flows,
                                           double end_value) {
  ch05::CashFlows cf{{0.0, -start}};
  for (std::size_t t = 0; t < flows.size(); ++t) {
    cf.push_back({static_cast<double>(t + 1), -flows[t]});
  }
  cf.push_back({static_cast<double>(flows.size() + 1), end_value});
  return ch05::InternalRateOfReturn(cf);
}

PerformanceStats Evaluate(std::span<const double> portfolio,
                          std::span<const double> market,
                          std::span<const double> riskfree) {
  const auto ep = Excess(portfolio, riskfree);
  const auto em = Excess(market, riskfree);
  const auto fit = stats::Ols(em, ep);
  PerformanceStats s{};
  s.mean_excess = stats::Mean(ep);
  s.std_dev = stats::StdDev(portfolio);
  s.beta = fit.beta;
  s.alpha = fit.alpha;
  s.alpha_t_stat =
      fit.alpha_std_error > 0.0 ? fit.alpha / fit.alpha_std_error : 0.0;
  s.residual_std_dev = fit.residual_std_dev;
  s.sharpe = s.mean_excess / s.std_dev;
  s.treynor = s.mean_excess / s.beta;
  const double sd_m = stats::StdDev(market);
  s.m_squared = (s.sharpe - stats::Mean(em) / sd_m) * sd_m;
  s.appraisal_ratio =
      s.residual_std_dev > 0.0 ? s.alpha / s.residual_std_dev : 0.0;
  return s;
}

double InformationRatio(std::span<const double> portfolio,
                        std::span<const double> benchmark) {
  const auto active = Excess(portfolio, benchmark);
  const double te = stats::StdDev(active);
  INVESTM_CHECK(te > 0.0, "tracking error is zero");
  return stats::Mean(active) / te;
}

std::optional<TimingFit> TreynorMazuy(std::span<const double> portfolio,
                                      std::span<const double> market,
                                      std::span<const double> riskfree) {
  return FitTiming(portfolio, market, riskfree, true);
}

std::optional<TimingFit> HenrikssonMerton(std::span<const double> portfolio,
                                          std::span<const double> market,
                                          std::span<const double> riskfree) {
  return FitTiming(portfolio, market, riskfree, false);
}

Attribution BrinsonAttribution(std::span<const double> portfolio_weights,
                               std::span<const double> benchmark_weights,
                               std::span<const double> portfolio_returns,
                               std::span<const double> benchmark_returns) {
  const std::size_t n = portfolio_weights.size();
  INVESTM_CHECK(benchmark_weights.size() == n &&
                    portfolio_returns.size() == n &&
                    benchmark_returns.size() == n,
                "size mismatch");
  Attribution a;
  a.portfolio_return = Dot(portfolio_weights, portfolio_returns);
  a.benchmark_return = Dot(benchmark_weights, benchmark_returns);
  for (std::size_t i = 0; i < n; ++i) {
    const double dw = portfolio_weights[i] - benchmark_weights[i];
    const double dr = portfolio_returns[i] - benchmark_returns[i];
    a.allocation.push_back(dw * (benchmark_returns[i] - a.benchmark_return));
    a.selection.push_back(benchmark_weights[i] * dr);
    a.interaction.push_back(dw * dr);
  }
  return a;
}

StyleFit StyleAnalysis(std::span<const double> fund, const Matrix& styles) {
  INVESTM_CHECK(fund.size() == styles.rows(), "size mismatch");
  INVESTM_CHECK(styles.cols() >= 1, "need at least one style index");
  // Minimize w' S w - 2 w' c (+ const), S = Cov(styles), c = Cov(styles,
  // fund), over the simplex: projected gradient with Nesterov momentum.
  const Matrix s = CovarianceMatrix(styles);
  const std::size_t k = styles.cols();
  Vector c(k);
  for (std::size_t i = 0; i < k; ++i) {
    c[i] = stats::Covariance(styles.Column(i), fund);
  }
  const double lipschitz = 2.0 * EigenSymmetric(s).values.front();
  INVESTM_CHECK(lipschitz > 0.0, "style indexes have no variance");
  const double step = 1.0 / lipschitz;
  Vector w(k, 1.0 / static_cast<double>(k));
  Vector y = w;
  double t = 1.0;
  for (int it = 0; it < 50000; ++it) {
    const Vector sy = s * y;
    Vector g(k);
    for (std::size_t i = 0; i < k; ++i)
      g[i] = y[i] - step * 2.0 * (sy[i] - c[i]);
    const Vector next = ch07::ProjectOntoSimplex(g);
    const double t_next = 0.5 * (1.0 + std::sqrt(1.0 + 4.0 * t * t));
    double change = 0.0;
    for (std::size_t i = 0; i < k; ++i) {
      y[i] = next[i] + (t - 1.0) / t_next * (next[i] - w[i]);
      change = std::max(change, std::abs(next[i] - w[i]));
    }
    w = next;
    t = t_next;
    if (change < 1e-13) break;
  }
  std::vector<double> resid(fund.size());
  for (std::size_t r = 0; r < fund.size(); ++r) {
    resid[r] = fund[r] - Dot(styles.Row(r), w);
  }
  return {w, 1.0 - stats::Variance(resid) / stats::Variance(fund)};
}

}  // namespace investm::ch24
