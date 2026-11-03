#include "cpp/ch04_efficient_markets/efficiency_tests.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

#include "cpp/core/check.h"
#include "cpp/core/stats.h"

namespace investm::ch04 {

std::vector<double> RandomWalkPrices(double p0, std::size_t n, double mu,
                                     double sigma, std::uint64_t seed) {
  std::mt19937_64 rng(seed);
  std::normal_distribution<double> z(mu, sigma);
  std::vector<double> prices;
  prices.reserve(n + 1);
  prices.push_back(p0);
  for (std::size_t i = 0; i < n; ++i) {
    prices.push_back(prices.back() * std::exp(z(rng)));
  }
  return prices;
}

std::vector<double> LogReturns(std::span<const double> prices) {
  INVESTM_CHECK(prices.size() >= 2, "need at least two prices");
  std::vector<double> r;
  r.reserve(prices.size() - 1);
  for (std::size_t i = 1; i < prices.size(); ++i) {
    r.push_back(std::log(prices[i] / prices[i - 1]));
  }
  return r;
}

RunsTestResult RunsTest(std::span<const double> returns) {
  std::vector<int> signs;
  signs.reserve(returns.size());
  for (double r : returns) {
    if (r != 0.0) signs.push_back(r > 0.0 ? 1 : -1);
  }
  INVESTM_CHECK(signs.size() >= 2, "too few non-zero returns");
  double n1 = 0.0;
  double n2 = 0.0;
  for (int s : signs) (s > 0 ? n1 : n2) += 1.0;
  std::size_t runs = 1;
  for (std::size_t i = 1; i < signs.size(); ++i) {
    if (signs[i] != signs[i - 1]) ++runs;
  }
  const double n = n1 + n2;
  const double mean = 2.0 * n1 * n2 / n + 1.0;
  const double var = 2.0 * n1 * n2 * (2.0 * n1 * n2 - n) / (n * n * (n - 1.0));
  const double z =
      var > 0.0 ? (static_cast<double>(runs) - mean) / std::sqrt(var) : 0.0;
  return {runs, mean, z};
}

double VarianceRatio(std::span<const double> log_returns, std::size_t q) {
  INVESTM_CHECK(q >= 2 && log_returns.size() > q, "invalid horizon q");
  // Rolling sums: O(n) instead of O(n q).
  std::vector<double> sums;
  sums.reserve(log_returns.size() - q + 1);
  double window = 0.0;
  for (std::size_t t = 0; t < log_returns.size(); ++t) {
    window += log_returns[t];
    if (t >= q) window -= log_returns[t - q];
    if (t + 1 >= q) sums.push_back(window);
  }
  return stats::Variance(sums) /
         (static_cast<double>(q) * stats::Variance(log_returns));
}

double FilterRuleReturn(std::span<const double> prices, double x) {
  INVESTM_CHECK(prices.size() >= 2, "need at least two prices");
  bool invested = false;
  double wealth = 1.0;
  double extreme = prices[0];
  double entry = 0.0;
  for (std::size_t t = 1; t < prices.size(); ++t) {
    const double p = prices[t];
    if (!invested) {
      extreme = std::min(extreme, p);
      if (p >= extreme * (1.0 + x)) {
        invested = true;
        entry = p;
        extreme = p;
      }
    } else {
      extreme = std::max(extreme, p);
      if (p <= extreme * (1.0 - x)) {
        invested = false;
        wealth *= p / entry;
        extreme = p;
      }
    }
  }
  if (invested) wealth *= prices.back() / entry;
  return wealth - 1.0;
}

EventStudyResult EventStudy(std::span<const EventFirm> firms,
                            std::size_t estimation_length) {
  INVESTM_CHECK(!firms.empty(), "no firms");
  const std::size_t total = firms.front().stock.size();
  INVESTM_CHECK(estimation_length < total, "no event window");
  const std::size_t event_length = total - estimation_length;

  EventStudyResult result;
  result.aar.assign(event_length, 0.0);
  double residual_var_sum = 0.0;
  for (const EventFirm& firm : firms) {
    INVESTM_CHECK(firm.stock.size() == total && firm.market.size() == total,
                  "inconsistent series lengths");
    const std::span<const double> stock(firm.stock);
    const std::span<const double> market(firm.market);
    const stats::SimpleRegression mm = stats::Ols(
        market.first(estimation_length), stock.first(estimation_length));
    residual_var_sum += mm.residual_std_dev * mm.residual_std_dev;
    for (std::size_t t = 0; t < event_length; ++t) {
      const std::size_t d = estimation_length + t;
      result.aar[t] += stock[d] - mm.Predict(market[d]);
    }
  }
  const double n = static_cast<double>(firms.size());
  double cumulative = 0.0;
  result.caar.reserve(event_length);
  for (double& a : result.aar) {
    a /= n;
    cumulative += a;
    result.caar.push_back(cumulative);
  }
  // Var(AAR_t) = sum(sigma_i^2) / N^2 and Var(CAAR) = L * Var(AAR).
  const double sd_caar =
      std::sqrt(static_cast<double>(event_length) * residual_var_sum / (n * n));
  result.caar_t_stat = cumulative / sd_caar;
  return result;
}

}  // namespace investm::ch04
