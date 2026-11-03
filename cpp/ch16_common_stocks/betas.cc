#include "cpp/ch16_common_stocks/betas.h"

#include <vector>

#include "cpp/core/check.h"

namespace investm::ch16 {

stats::SimpleRegression EstimateBeta(std::span<const double> security,
                                     std::span<const double> market) {
  return stats::Ols(market, security);
}

std::vector<double> RollingBetas(std::span<const double> security,
                                 std::span<const double> market,
                                 std::size_t window) {
  INVESTM_CHECK(security.size() == market.size(), "size mismatch");
  INVESTM_CHECK(window >= 3, "window must hold at least 3 observations");
  std::vector<double> betas;
  for (std::size_t start = 0; start + window <= security.size();
       start += window) {
    betas.push_back(stats::Ols(market.subspan(start, window),
                               security.subspan(start, window))
                        .beta);
  }
  return betas;
}

double ShrinkBeta(double raw, double weight, double prior) {
  INVESTM_CHECK(weight >= 0.0 && weight <= 1.0, "weight in [0, 1]");
  return weight * raw + (1.0 - weight) * prior;
}

double VasicekBeta(double raw, double raw_std_error, double prior_mean,
                   double prior_std_dev) {
  INVESTM_CHECK(raw_std_error > 0.0 && prior_std_dev > 0.0,
                "standard deviations must be positive");
  const double v_raw = raw_std_error * raw_std_error;
  const double v_prior = prior_std_dev * prior_std_dev;
  return (v_raw * prior_mean + v_prior * raw) / (v_raw + v_prior);
}

stats::SimpleRegression BetaPersistence(std::span<const double> earlier,
                                        std::span<const double> later) {
  return stats::Ols(earlier, later);
}

double LeveredBeta(double unlevered_beta, double debt_to_equity,
                   double tax_rate) {
  INVESTM_CHECK(debt_to_equity >= 0.0, "D/E must be non-negative");
  return unlevered_beta * (1.0 + (1.0 - tax_rate) * debt_to_equity);
}

double UnleveredBeta(double levered_beta, double debt_to_equity,
                     double tax_rate) {
  INVESTM_CHECK(debt_to_equity >= 0.0, "D/E must be non-negative");
  return levered_beta / (1.0 + (1.0 - tax_rate) * debt_to_equity);
}

}  // namespace investm::ch16
