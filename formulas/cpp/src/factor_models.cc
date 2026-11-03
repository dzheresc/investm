#include "investm/formulas/factor_models.h"

namespace investm::formulas {

double TwoFactorReturn(double a, double b1, double f1, double b2, double f2,
                       double error) {
  return a + b1 * f1 + b2 * f2 + error;
}

double TwoFactorVariance(double b1, double b2, double var_f1, double var_f2,
                         double cov_f1_f2, double residual_variance) {
  return b1 * b1 * var_f1 + b2 * b2 * var_f2 + 2.0 * b1 * b2 * cov_f1_f2 +
         residual_variance;
}

double TwoFactorCovariance(double b_i1, double b_i2, double b_j1, double b_j2,
                           double var_f1, double var_f2, double cov_f1_f2) {
  return b_i1 * b_j1 * var_f1 + b_i2 * b_j2 * var_f2 +
         (b_i1 * b_j2 + b_i2 * b_j1) * cov_f1_f2;
}

double AptExpectedReturn(double lambda0, double lambda1, double lambda2,
                         double b1, double b2) {
  return lambda0 + lambda1 * b1 + lambda2 * b2;
}

double AptExpectedReturnFromFactorPortfolios(double risk_free, double delta1,
                                             double delta2, double b1,
                                             double b2) {
  return risk_free + (delta1 - risk_free) * b1 + (delta2 - risk_free) * b2;
}

}  // namespace investm::formulas
