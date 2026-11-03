// Chapter 18, Earnings.
// The link between earnings and dividends (Lintner's partial adjustment
// model), naive time-series forecasts of earnings, standardized unexpected
// earnings, and sorting stocks into portfolios by earnings surprise.

#ifndef CPP_CH18_EARNINGS_EARNINGS_H_
#define CPP_CH18_EARNINGS_EARNINGS_H_

#include <optional>
#include <span>
#include <vector>

namespace investm::ch18 {

// 18.3 Lintner's model: firms move dividends only part of the way toward a
// target payout of current earnings,
//   D_t - D_{t-1} = a + c (p* E_t - D_{t-1}) + e_t.
struct LintnerModel {
  double a;              // Intercept.
  double adjustment;     // c: speed of adjustment, in (0, 1].
  double target_payout;  // p*.
  double r_squared;

  double NextDividend(double previous_dividend, double earnings) const {
    return previous_dividend + a +
           adjustment * (target_payout * earnings - previous_dividend);
  }
};
// Fits the model by regressing dD_t on E_t and D_{t-1}. `earnings[t]` and
// `dividends[t]` are for year t; needs at least 4 years. nullopt if the
// regression is singular or implies no adjustment (c <= 0).
std::optional<LintnerModel> FitLintner(std::span<const double> earnings,
                                       std::span<const double> dividends);

// 18.4 Naive earnings forecasts.
// Annual: random walk with drift, E_{T+1} = E_T + mean(E_t - E_{t-1}).
double RandomWalkForecast(std::span<const double> annual_earnings);
// Quarterly: seasonal random walk with drift,
//   E_q = E_{q-4} + mean of past seasonal differences.
double SeasonalRandomWalkForecast(std::span<const double> quarterly_earnings);

// 18.5 Standardized unexpected earnings: (actual - forecast) / sd, where sd
// is the standard deviation of past seasonal differences (the forecast
// errors of the seasonal random walk).
double StandardizedUnexpectedEarnings(std::span<const double> quarterly_history,
                                      double actual);

// Sorts items into `groups` portfolios of (nearly) equal size by score,
// lowest scores in group 0. Returns the group of each item.
std::vector<int> SortIntoGroups(std::span<const double> scores, int groups);
// Mean of `values` within each group.
std::vector<double> GroupMeans(std::span<const double> values,
                               std::span<const int> group, int groups);

}  // namespace investm::ch18

#endif  // CPP_CH18_EARNINGS_EARNINGS_H_
