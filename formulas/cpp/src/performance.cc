#include "investm/formulas/performance.h"

#include <cmath>

namespace investm::formulas {

double NetAssetValue(double market_value_assets, double liabilities,
                     double shares_outstanding) {
  return (market_value_assets - liabilities) / shares_outstanding;
}

double InvestmentCompanyReturn(double nav, double previous_nav, double income,
                               double capital_gains) {
  return (nav - previous_nav + income + capital_gains) / previous_nav;
}

double ImpliedRiskTolerance(double chosen_return, double stock_return,
                            double risk_free, double stock_std_dev) {
  const double excess = stock_return - risk_free;
  return 2.0 * (chosen_return - risk_free) * stock_std_dev * stock_std_dev /
         (excess * excess);
}

double CertaintyEquivalentReturn(double expected_return, double std_dev,
                                 double risk_tolerance) {
  return expected_return - std_dev * std_dev / risk_tolerance;
}

double ExPostAlpha(double portfolio_return, double benchmark_return) {
  return portfolio_return - benchmark_return;
}

double CapmExPostAlpha(double portfolio_return, double risk_free,
                       double market_return, double beta) {
  return portfolio_return - (risk_free + (market_return - risk_free) * beta);
}

double CharacteristicLine(double alpha, double beta, double market_return,
                          double risk_free) {
  return alpha + beta * (market_return - risk_free);
}

double TreynorRatio(double portfolio_return, double risk_free, double beta) {
  return (portfolio_return - risk_free) / beta;
}

double SharpeRatio(double portfolio_return, double risk_free, double std_dev) {
  return (portfolio_return - risk_free) / std_dev;
}

double MSquared(double portfolio_return, double risk_free, double std_dev,
                double market_std_dev) {
  return risk_free +
         SharpeRatio(portfolio_return, risk_free, std_dev) * market_std_dev;
}

double QuadraticCharacteristicCurve(double a, double b, double c,
                                    double market_return, double risk_free) {
  const double excess = market_return - risk_free;
  return a + b * excess + c * excess * excess;
}

double DummyCharacteristicLines(double a, double b, double c,
                                double market_return, double risk_free) {
  const double excess = market_return - risk_free;
  const double dummy = market_return > risk_free ? 0.0 : -1.0;
  return a + b * excess + c * dummy * excess;
}

double ForeignReturn(double local_return, double currency_return) {
  return local_return + currency_return + local_return * currency_return;
}

double ForeignStdDev(double local_std_dev, double currency_std_dev,
                     double correlation) {
  return std::sqrt(local_std_dev * local_std_dev +
                   currency_std_dev * currency_std_dev +
                   2.0 * correlation * local_std_dev * currency_std_dev);
}

}  // namespace investm::formulas
