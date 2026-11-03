// Chapter 24, Portfolio Performance Evaluation.
// Measuring returns (time-weighted vs dollar-weighted), risk-adjusted
// measures (Sharpe, Treynor, Jensen's alpha, M^2, appraisal and information
// ratios), market-timing tests, Brinson performance attribution and
// returns-based style analysis.

#ifndef CPP_CH24_PERFORMANCE_PERFORMANCE_H_
#define CPP_CH24_PERFORMANCE_PERFORMANCE_H_

#include <optional>
#include <span>
#include <vector>

#include "cpp/ch05_riskless_securities/cash_flows.h"
#include "cpp/core/matrix.h"

namespace investm::ch24 {

// 24.2 Time-weighted return: chain-linked subperiod returns.
double TimeWeightedReturn(std::span<const double> period_returns);
// Subperiod returns from portfolio values: value_before[t] is the value
// at the end of period t just before an external flow flows[t] (positive
// = deposit) at that date; start is the initial value.
std::vector<double> PeriodReturnsFromValues(double start,
                                            std::span<const double> value_end,
                                            std::span<const double> flows);
// Dollar-weighted return: the IRR per period of -start, -flows, +end.
std::optional<double> DollarWeightedReturn(double start,
                                           std::span<const double> flows,
                                           double end_value);

// 24.4 Risk-adjusted measures. Returns are per period; excess returns are
// over the matching riskless rates.
struct PerformanceStats {
  double mean_excess;
  double std_dev;
  double beta;
  double alpha;  // Jensen's alpha (intercept of the excess SCL).
  double alpha_t_stat;
  double residual_std_dev;
  double sharpe;           // mean_excess / std_dev.
  double treynor;          // mean_excess / beta.
  double m_squared;        // (Sharpe_p - Sharpe_m) x sd_m.
  double appraisal_ratio;  // alpha / residual sd.
};
PerformanceStats Evaluate(std::span<const double> portfolio,
                          std::span<const double> market,
                          std::span<const double> riskfree);

// Information ratio of active returns (portfolio - benchmark):
// mean / tracking error.
double InformationRatio(std::span<const double> portfolio,
                        std::span<const double> benchmark);

// 24.6 Market timing. Treynor-Mazuy: e_p = a + b e_m + c e_m^2;
// Henriksson-Merton: e_p = a + b e_m + c max(0, -e_m). A positive c
// suggests timing ability.
struct TimingFit {
  double alpha;
  double beta;
  double timing;
  double timing_t_stat;
};
std::optional<TimingFit> TreynorMazuy(std::span<const double> portfolio,
                                      std::span<const double> market,
                                      std::span<const double> riskfree);
std::optional<TimingFit> HenrikssonMerton(std::span<const double> portfolio,
                                          std::span<const double> market,
                                          std::span<const double> riskfree);

// 24.7 Brinson attribution by segment.
struct Attribution {
  Vector allocation;   // (w_p - w_b)(r_b - R_b)
  Vector selection;    // w_b (r_p - r_b)
  Vector interaction;  // (w_p - w_b)(r_p - r_b)
  double portfolio_return;
  double benchmark_return;
  double Total() const { return portfolio_return - benchmark_return; }
};
Attribution BrinsonAttribution(std::span<const double> portfolio_weights,
                               std::span<const double> benchmark_weights,
                               std::span<const double> portfolio_returns,
                               std::span<const double> benchmark_returns);

// 24.8 Returns-based style analysis: non-negative weights summing to one
// on the style indexes (columns of `styles`) that minimize the variance of
// fund - styles x w.
struct StyleFit {
  Vector weights;
  double r_squared;  // 1 - var(residual) / var(fund).
};
StyleFit StyleAnalysis(std::span<const double> fund, const Matrix& styles);

}  // namespace investm::ch24

#endif  // CPP_CH24_PERFORMANCE_PERFORMANCE_H_
