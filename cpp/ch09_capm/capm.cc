#include "cpp/ch09_capm/capm.h"

#include "cpp/core/check.h"

namespace investm::ch09 {

double CapitalMarketLine(double rf, double market_return, double market_std_dev,
                         double std_dev) {
  INVESTM_CHECK(market_std_dev > 0.0, "market must be risky");
  return rf + (market_return - rf) / market_std_dev * std_dev;
}

double SecurityMarketLine(double rf, double market_return, double beta) {
  return rf + (market_return - rf) * beta;
}

double Beta(double cov_with_market, double market_variance) {
  INVESTM_CHECK(market_variance > 0.0, "market must be risky");
  return cov_with_market / market_variance;
}

Vector Betas(const Matrix& cov, std::span<const double> market_weights) {
  Vector cov_with_market = cov * market_weights;
  const double market_variance = Dot(market_weights, cov_with_market);
  INVESTM_CHECK(market_variance > 0.0, "market must be risky");
  for (double& c : cov_with_market) c /= market_variance;
  return cov_with_market;
}

double Alpha(double expected_return, double rf, double market_return,
             double beta) {
  return expected_return - SecurityMarketLine(rf, market_return, beta);
}

Vector ImpliedExpectedReturns(const Matrix& cov,
                              std::span<const double> market_weights, double rf,
                              double market_risk_premium) {
  Vector mu = Betas(cov, market_weights);
  for (double& m : mu) m = rf + m * market_risk_premium;
  return mu;
}

RiskPartition PartitionRisk(double total_variance, double beta,
                            double market_variance) {
  const double market = beta * beta * market_variance;
  return {market, total_variance - market};
}

double ZeroBetaReturn(double a, double b, double c, double frontier_return) {
  const double denom = a * frontier_return - b;
  INVESTM_CHECK(denom != 0.0,
                "the minimum-variance portfolio has no zero-beta partner");
  return (b * frontier_return - c) / denom;
}

double ZeroBetaSecurityMarketLine(double zero_beta_return, double market_return,
                                  double beta) {
  return zero_beta_return + (market_return - zero_beta_return) * beta;
}

double LiquidityAdjustedReturn(double rf, double market_return, double beta,
                               double trading_cost, double cost_premium) {
  return SecurityMarketLine(rf, market_return, beta) +
         cost_premium * trading_cost;
}

}  // namespace investm::ch09
