#include "investm/formulas/equity.h"

#include <cmath>
#include <cstddef>
#include <limits>
#include <span>

namespace investm::formulas {

double SharesToElectDirectors(double directors_wanted, double shares_voting,
                              double directors_elected) {
  return directors_wanted * shares_voting / (directors_elected + 1.0) + 1.0;
}

double RightValueRightsOn(double stock_price, double subscription_price,
                          double rights_per_share) {
  return (stock_price - subscription_price) / (rights_per_share + 1.0);
}

double RightValueExRights(double stock_price, double subscription_price,
                          double rights_per_share) {
  return (stock_price - subscription_price) / rights_per_share;
}

double FirmBeta(double beta_debt, double beta_equity, double tax_rate,
                double debt, double equity, double unlevered_value) {
  return beta_debt * (1.0 - tax_rate) * debt / unlevered_value +
         beta_equity * equity / unlevered_value;
}

double EquityBeta(double beta_firm, double beta_debt, double debt,
                  double equity, double tax_rate) {
  return beta_firm +
         (beta_firm - beta_debt) * (debt / equity) * (1.0 - tax_rate);
}

double AdjustedBeta(double a, double b, double historical_beta) {
  return a + b * historical_beta;
}

double CapitalizedValue(std::span<const double> cash_flows,
                        double discount_rate) {
  double value = 0.0;
  for (std::size_t t = 1; t <= cash_flows.size(); ++t) {
    value += cash_flows[t - 1] /
             std::pow(1.0 + discount_rate, static_cast<double>(t));
  }
  return value;
}

double ZeroGrowthDdm(double next_dividend, double discount_rate) {
  return next_dividend / discount_rate;
}

double ConstantGrowthDdm(double next_dividend, double discount_rate,
                         double growth) {
  if (discount_rate <= growth) return std::numeric_limits<double>::quiet_NaN();
  return next_dividend / (discount_rate - growth);
}

double MultipleGrowthDdm(std::span<const double> dividends,
                         double dividend_after, double discount_rate,
                         double growth) {
  if (discount_rate <= growth) return std::numeric_limits<double>::quiet_NaN();
  const double horizon = static_cast<double>(dividends.size());
  return CapitalizedValue(dividends, discount_rate) +
         dividend_after / ((discount_rate - growth) *
                           std::pow(1.0 + discount_rate, horizon));
}

double NormalPriceEarnings(double payout, double return_on_equity,
                           double discount_rate) {
  const double g = return_on_equity * (1.0 - payout);
  return payout * (1.0 + g) / (discount_rate - g);
}

double LintnerDividend(double adjustment, double target_payout, double earnings,
                       double previous_dividend) {
  return adjustment * target_payout * earnings +
         (1.0 - adjustment) * previous_dividend;
}

double RandomWalkEarnings(double previous_earnings, double shock) {
  return previous_earnings + shock;
}

double QuarterlyEarnings(double qe_lag4, double qe_lag1, double qe_lag5,
                         double a, double b, double error) {
  return qe_lag4 + a * (qe_lag1 - qe_lag5) + b + error;
}

}  // namespace investm::formulas
