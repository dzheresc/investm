// Investment companies, risk tolerance, performance evaluation and foreign
// investing: equations 61 to 74.

#ifndef INVESTM_FORMULAS_PERFORMANCE_H_
#define INVESTM_FORMULAS_PERFORMANCE_H_

namespace investm::formulas {

// Eq. 61: NAV_t = (MVA_t - LIAB_t) / NSO_t.
double NetAssetValue(double market_value_assets, double liabilities,
                     double shares_outstanding);

// Eq. 62: r_t = (NAV_t - NAV_{t-1} + I_t + G_t) / NAV_{t-1}.
double InvestmentCompanyReturn(double nav, double previous_nav, double income,
                               double capital_gains);

// Eq. 63: tau = 2 (r_C - r_f) sigma_S^2 / (r_S - r_f)^2.
double ImpliedRiskTolerance(double chosen_return, double stock_return,
                            double risk_free, double stock_std_dev);

// Eq. 64: u = r_p - (1 / tau) sigma_p^2.
double CertaintyEquivalentReturn(double expected_return, double std_dev,
                                 double risk_tolerance);

// Eq. 65: alpha_p = ar_p - ar_bp.
double ExPostAlpha(double portfolio_return, double benchmark_return);

// Eq. 66: alpha_p = ar_p - [ar_f + (ar_M - ar_f) beta_p].
double CapmExPostAlpha(double portfolio_return, double risk_free,
                       double market_return, double beta);

// Eq. 67: returns r_p - r_f = alpha_p + beta_p (r_M - r_f).
double CharacteristicLine(double alpha, double beta, double market_return,
                          double risk_free);

// Eq. 68: RVOL_p = (ar_p - ar_f) / beta_p.
double TreynorRatio(double portfolio_return, double risk_free, double beta);

// Eq. 69: SR_p = (ar_p - ar_f) / sigma_p.
double SharpeRatio(double portfolio_return, double risk_free, double std_dev);

// Eq. 70: M^2 = ar_f + ((ar_p - ar_f) / sigma_p) sigma_M.
double MSquared(double portfolio_return, double risk_free, double std_dev,
                double market_std_dev);

// Eq. 71: returns a + b (r_M - r_f) + c (r_M - r_f)^2.
double QuadraticCharacteristicCurve(double a, double b, double c,
                                    double market_return, double risk_free);

// Eq. 72: returns a + b (r_M - r_f) + c D (r_M - r_f), where D = 0 when
// r_M > r_f and D = -1 otherwise.
double DummyCharacteristicLines(double a, double b, double c,
                                double market_return, double risk_free);

// Eq. 73: r_F = r_d + r_e + r_d r_e.
double ForeignReturn(double local_return, double currency_return);

// Eq. 74: sigma_F = (sigma_d^2 + sigma_e^2 + 2 rho sigma_d sigma_e)^(1/2).
double ForeignStdDev(double local_std_dev, double currency_std_dev,
                     double correlation);

}  // namespace investm::formulas

#endif  // INVESTM_FORMULAS_PERFORMANCE_H_
