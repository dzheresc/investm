// Two-factor models and the Arbitrage Pricing Theory: equations 21 to 24.

#ifndef INVESTM_FORMULAS_FACTOR_MODELS_H_
#define INVESTM_FORMULAS_FACTOR_MODELS_H_

namespace investm::formulas {

// Eq. 21: r_i = a_i + b_i1 F_1 + b_i2 F_2 + e_i.
double TwoFactorReturn(double a, double b1, double f1, double b2, double f2,
                       double error = 0.0);

// Eq. 22: variance of a security under a two-factor model.
double TwoFactorVariance(double b1, double b2, double var_f1, double var_f2,
                         double cov_f1_f2, double residual_variance);

// Eq. 23: covariance between two securities under a two-factor model.
double TwoFactorCovariance(double b_i1, double b_i2, double b_j1, double b_j2,
                           double var_f1, double var_f2, double cov_f1_f2);

// Eq. 24, first form: r_i = lambda_0 + lambda_1 b_i1 + lambda_2 b_i2.
double AptExpectedReturn(double lambda0, double lambda1, double lambda2,
                         double b1, double b2);

// Eq. 24, second form: r_f + (delta_1 - r_f) b_i1 + (delta_2 - r_f) b_i2.
double AptExpectedReturnFromFactorPortfolios(double risk_free, double delta1,
                                             double delta2, double b1,
                                             double b2);

}  // namespace investm::formulas

#endif  // INVESTM_FORMULAS_FACTOR_MODELS_H_
