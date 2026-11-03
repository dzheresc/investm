// Chapter 22, Financial Analysis.
// Technical analysis: moving averages and crossover rules, momentum,
// relative strength, and a backtest of a moving-average timing rule.

#ifndef CPP_CH22_FINANCIAL_ANALYSIS_TECHNICAL_H_
#define CPP_CH22_FINANCIAL_ANALYSIS_TECHNICAL_H_

#include <span>
#include <vector>

namespace investm::ch22 {

// Trailing simple moving average; element t averages prices t-n+1..t.
// The first n-1 elements are NaN.
std::vector<double> SimpleMovingAverage(std::span<const double> prices,
                                        std::size_t n);

// +1 when price > its n-period moving average, -1 when below, 0 while the
// average is undefined or equal.
std::vector<int> MovingAverageSignal(std::span<const double> prices,
                                     std::size_t n);

// Rate of change over `lag` periods: P_t / P_{t-lag} - 1 (NaN for t < lag).
std::vector<double> Momentum(std::span<const double> prices, std::size_t lag);

// Relative strength: stock price / index level, rebased to 1 at t = 0.
std::vector<double> RelativeStrength(std::span<const double> stock,
                                     std::span<const double> index);

// Backtest: hold the asset in period t+1 when the signal at t is +1,
// otherwise earn `cash_rate` per period. Returns per-period returns
// (one fewer than prices).
std::vector<double> MovingAverageRuleReturns(std::span<const double> prices,
                                             std::size_t n, double cash_rate);

}  // namespace investm::ch22

#endif  // CPP_CH22_FINANCIAL_ANALYSIS_TECHNICAL_H_
