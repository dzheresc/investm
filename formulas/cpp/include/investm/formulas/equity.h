// Voting, rights, betas of the firm and equity valuation: equations 35 to 37
// and 44 to 54.

#ifndef INVESTM_FORMULAS_EQUITY_H_
#define INVESTM_FORMULAS_EQUITY_H_

#include <span>

namespace investm::formulas {

// Eq. 35: n = d s / (D + 1) + 1 shares under cumulative voting.
double SharesToElectDirectors(double directors_wanted, double shares_voting,
                              double directors_elected);

// Eq. 36: R = (C_o - S) / (N + 1).
double RightValueRightsOn(double stock_price, double subscription_price,
                          double rights_per_share);

// Eq. 37: R = (C_e - S) / N.
double RightValueExRights(double stock_price, double subscription_price,
                          double rights_per_share);

// Eq. 44: beta_firm = beta_debt (1 - tau) D / V_u + beta_equity E / V_u.
double FirmBeta(double beta_debt, double beta_equity, double tax_rate,
                double debt, double equity, double unlevered_value);

// Eq. 45: beta_equity = beta_firm + (beta_firm - beta_debt)(D/E)(1 - tau).
double EquityBeta(double beta_firm, double beta_debt, double debt,
                  double equity, double tax_rate);

// Eq. 46: beta_a = a + b beta_h.
double AdjustedBeta(double a, double b, double historical_beta);

// Eq. 47: V = sum C_t / (1 + k)^t for a finite stream C_1..C_T.
double CapitalizedValue(std::span<const double> cash_flows,
                        double discount_rate);

// Eq. 48: V = D_1 / k.
double ZeroGrowthDdm(double next_dividend, double discount_rate);

// Eq. 49: V = D_1 / (k - g). Returns NaN unless k > g.
double ConstantGrowthDdm(double next_dividend, double discount_rate,
                         double growth);

// Eq. 50: PV of D_1..D_T plus D_{T+1} / ((k - g)(1 + k)^T).
// Returns NaN unless k > g.
double MultipleGrowthDdm(std::span<const double> dividends,
                         double dividend_after, double discount_rate,
                         double growth);

// Eq. 51: V / E_0 = p (1 + r(1 - p)) / (k - r(1 - p)).
double NormalPriceEarnings(double payout, double return_on_equity,
                           double discount_rate);

// Eq. 52: D_t = a p* E_t + (1 - a) D_{t-1}.
double LintnerDividend(double adjustment, double target_payout, double earnings,
                       double previous_dividend);

// Eq. 53: E_t = E_{t-1} + epsilon_t.
double RandomWalkEarnings(double previous_earnings, double shock);

// Eq. 54: QE_t = QE_{t-4} + a (QE_{t-1} - QE_{t-5}) + b + e_t.
double QuarterlyEarnings(double qe_lag4, double qe_lag1, double qe_lag5,
                         double a, double b, double error = 0.0);

}  // namespace investm::formulas

#endif  // INVESTM_FORMULAS_EQUITY_H_
