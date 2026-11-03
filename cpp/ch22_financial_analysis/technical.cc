#include "cpp/ch22_financial_analysis/technical.h"

#include <cmath>
#include <limits>
#include <vector>

#include "cpp/core/check.h"

namespace investm::ch22 {
namespace {

constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();

}  // namespace

std::vector<double> SimpleMovingAverage(std::span<const double> prices,
                                        std::size_t n) {
  INVESTM_CHECK(n >= 1, "window must be at least 1");
  std::vector<double> ma(prices.size(), kNaN);
  double sum = 0.0;
  for (std::size_t t = 0; t < prices.size(); ++t) {
    sum += prices[t];
    if (t >= n) sum -= prices[t - n];
    if (t + 1 >= n) ma[t] = sum / static_cast<double>(n);
  }
  return ma;
}

std::vector<int> MovingAverageSignal(std::span<const double> prices,
                                     std::size_t n) {
  const auto ma = SimpleMovingAverage(prices, n);
  std::vector<int> signal(prices.size(), 0);
  for (std::size_t t = 0; t < prices.size(); ++t) {
    if (std::isnan(ma[t])) continue;
    if (prices[t] > ma[t]) {
      signal[t] = 1;
    } else if (prices[t] < ma[t]) {
      signal[t] = -1;
    }
  }
  return signal;
}

std::vector<double> Momentum(std::span<const double> prices, std::size_t lag) {
  INVESTM_CHECK(lag >= 1, "lag must be at least 1");
  std::vector<double> m(prices.size(), kNaN);
  for (std::size_t t = lag; t < prices.size(); ++t) {
    m[t] = prices[t] / prices[t - lag] - 1.0;
  }
  return m;
}

std::vector<double> RelativeStrength(std::span<const double> stock,
                                     std::span<const double> index) {
  INVESTM_CHECK(stock.size() == index.size() && !stock.empty(),
                "size mismatch");
  std::vector<double> rs(stock.size());
  const double base = stock[0] / index[0];
  for (std::size_t t = 0; t < stock.size(); ++t) {
    rs[t] = stock[t] / index[t] / base;
  }
  return rs;
}

std::vector<double> MovingAverageRuleReturns(std::span<const double> prices,
                                             std::size_t n, double cash_rate) {
  INVESTM_CHECK(prices.size() >= 2, "need at least two prices");
  const auto signal = MovingAverageSignal(prices, n);
  std::vector<double> r(prices.size() - 1);
  for (std::size_t t = 0; t + 1 < prices.size(); ++t) {
    r[t] = signal[t] == 1 ? prices[t + 1] / prices[t] - 1.0 : cash_rate;
  }
  return r;
}

}  // namespace investm::ch22
