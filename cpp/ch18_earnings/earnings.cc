#include "cpp/ch18_earnings/earnings.h"

#include <algorithm>
#include <numeric>
#include <vector>

#include "cpp/core/check.h"
#include "cpp/core/matrix.h"
#include "cpp/core/regression.h"
#include "cpp/core/stats.h"

namespace investm::ch18 {
namespace {

constexpr std::size_t kSeason = 4;

std::vector<double> SeasonalDifferences(std::span<const double> q) {
  INVESTM_CHECK(q.size() >= kSeason + 2,
                "need at least six quarters of history");
  std::vector<double> diff;
  diff.reserve(q.size() - kSeason);
  for (std::size_t t = kSeason; t < q.size(); ++t) {
    diff.push_back(q[t] - q[t - kSeason]);
  }
  return diff;
}

}  // namespace

std::optional<LintnerModel> FitLintner(std::span<const double> earnings,
                                       std::span<const double> dividends) {
  INVESTM_CHECK(earnings.size() == dividends.size(), "size mismatch");
  INVESTM_CHECK(earnings.size() >= 5, "need at least five years");
  const std::size_t n = earnings.size() - 1;
  Matrix x(n, 2);
  Vector y(n);
  for (std::size_t t = 1; t <= n; ++t) {
    x(t - 1, 0) = earnings[t];
    x(t - 1, 1) = dividends[t - 1];
    y[t - 1] = dividends[t] - dividends[t - 1];
  }
  const auto fit = MultipleOls(x, y);
  if (!fit) return std::nullopt;
  const double c = -fit->coefficients[2];
  if (c <= 0.0) return std::nullopt;
  return LintnerModel{fit->coefficients[0], c, fit->coefficients[1] / c,
                      fit->r_squared};
}

double RandomWalkForecast(std::span<const double> annual_earnings) {
  INVESTM_CHECK(annual_earnings.size() >= 2, "need two years of earnings");
  const double drift = (annual_earnings.back() - annual_earnings.front()) /
                       static_cast<double>(annual_earnings.size() - 1);
  return annual_earnings.back() + drift;
}

double SeasonalRandomWalkForecast(std::span<const double> quarterly_earnings) {
  const auto diff = SeasonalDifferences(quarterly_earnings);
  return quarterly_earnings[quarterly_earnings.size() - kSeason] +
         stats::Mean(diff);
}

double StandardizedUnexpectedEarnings(std::span<const double> quarterly_history,
                                      double actual) {
  const auto diff = SeasonalDifferences(quarterly_history);
  const double sd = stats::StdDev(diff);
  INVESTM_CHECK(sd > 0.0, "seasonal differences have no variation");
  return (actual - SeasonalRandomWalkForecast(quarterly_history)) / sd;
}

std::vector<int> SortIntoGroups(std::span<const double> scores, int groups) {
  INVESTM_CHECK(groups > 0, "need at least one group");
  INVESTM_CHECK(scores.size() >= static_cast<std::size_t>(groups),
                "fewer items than groups");
  std::vector<std::size_t> order(scores.size());
  std::iota(order.begin(), order.end(), std::size_t{0});
  std::stable_sort(
      order.begin(), order.end(),
      [&](std::size_t a, std::size_t b) { return scores[a] < scores[b]; });
  std::vector<int> group(scores.size());
  const std::size_t n = scores.size();
  for (std::size_t rank = 0; rank < n; ++rank) {
    group[order[rank]] =
        static_cast<int>(rank * static_cast<std::size_t>(groups) / n);
  }
  return group;
}

std::vector<double> GroupMeans(std::span<const double> values,
                               std::span<const int> group, int groups) {
  INVESTM_CHECK(values.size() == group.size(), "size mismatch");
  const auto g = static_cast<std::size_t>(groups);
  std::vector<double> sum(g, 0.0);
  std::vector<double> count(g, 0.0);
  for (std::size_t i = 0; i < values.size(); ++i) {
    INVESTM_CHECK(group[i] >= 0 && group[i] < groups, "group out of range");
    sum[static_cast<std::size_t>(group[i])] += values[i];
    count[static_cast<std::size_t>(group[i])] += 1.0;
  }
  for (std::size_t k = 0; k < g; ++k) {
    sum[k] = count[k] > 0.0 ? sum[k] / count[k] : 0.0;
  }
  return sum;
}

}  // namespace investm::ch18
