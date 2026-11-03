#include "cpp/ch01_introduction/returns.h"

#include <cmath>
#include <vector>

#include "cpp/core/check.h"

namespace investm::ch01 {

double HoldingPeriodReturn(double begin_value, double end_value,
                           double income) {
  INVESTM_CHECK(begin_value > 0.0, "begin value must be positive");
  return (end_value + income - begin_value) / begin_value;
}

std::vector<double> SimpleReturns(std::span<const double> prices,
                                  std::span<const double> income) {
  INVESTM_CHECK(prices.size() >= 2, "need at least two prices");
  INVESTM_CHECK(income.empty() || income.size() == prices.size() - 1,
                "income must have prices.size() - 1 entries");
  std::vector<double> r;
  r.reserve(prices.size() - 1);
  for (std::size_t t = 1; t < prices.size(); ++t) {
    r.push_back(HoldingPeriodReturn(prices[t - 1], prices[t],
                                    income.empty() ? 0.0 : income[t - 1]));
  }
  return r;
}

double CumulativeReturn(std::span<const double> returns) {
  double growth = 1.0;
  for (double r : returns) growth *= 1.0 + r;
  return growth - 1.0;
}

double Annualize(double cumulative, double years) {
  INVESTM_CHECK(years > 0.0, "years must be positive");
  return std::pow(1.0 + cumulative, 1.0 / years) - 1.0;
}

double EqualWeightPortfolioStdDev(int n_assets, double sigma, double rho) {
  INVESTM_CHECK(n_assets >= 1, "need at least one asset");
  const double n = n_assets;
  const double variance =
      sigma * sigma / n + (1.0 - 1.0 / n) * rho * sigma * sigma;
  return std::sqrt(variance);
}

}  // namespace investm::ch01
