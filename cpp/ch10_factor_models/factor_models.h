// Chapter 10, Factor Models.
// Sections 10.1-10.3 (one-factor, two-factor, sector-factor and general
// multiple-factor models), 10.4 (estimating factor models: time-series,
// cross-sectional and factor-analytic approaches).
//
// A K-factor return-generating process for N securities:
//   r_i = a_i + b_i1 F_1 + ... + b_iK F_K + e_i,
// with residuals uncorrelated with the factors and with each other. Its
// implied moments are
//   E[r] = a + B E[F],   Cov(r) = B S_F B' + diag(sigma_e^2).

#ifndef CPP_CH10_FACTOR_MODELS_FACTOR_MODELS_H_
#define CPP_CH10_FACTOR_MODELS_FACTOR_MODELS_H_

#include <optional>
#include <span>
#include <vector>

#include "cpp/core/matrix.h"

namespace investm::ch10 {

struct FactorModel {
  Vector intercepts;          // a, length N.
  Matrix sensitivities;       // B, N x K.
  Vector residual_variances;  // sigma_e^2, length N.

  std::size_t num_securities() const { return intercepts.size(); }
  std::size_t num_factors() const { return sensitivities.cols(); }
};

// Implied expected returns a + B E[F].
Vector FactorExpectedReturns(const FactorModel& model,
                             std::span<const double> factor_means);

// Implied covariance matrix B S_F B' + D.
Matrix FactorCovariance(const FactorModel& model, const Matrix& factor_cov);

// A portfolio's sensitivity to each factor: b_p = B' w.
Vector PortfolioSensitivities(const FactorModel& model,
                              std::span<const double> weights);

// Factor (systematic) and residual (unique) parts of a portfolio's variance.
struct VarianceDecomposition {
  double factor_variance;
  double residual_variance;
};
VarianceDecomposition DecomposePortfolioVariance(
    const FactorModel& model, const Matrix& factor_cov,
    std::span<const double> weights);

// 10.4.1 Time-series approach: regress each security's returns (columns of
// `returns`, T x N) on observed factor values (`factors`, T x K).
std::optional<FactorModel> EstimateTimeSeries(const Matrix& returns,
                                              const Matrix& factors);

// 10.4.2 Cross-sectional approach: in each period, regress the N security
// returns on their known attributes (N x K). The fitted slopes are that
// period's factor values; the intercept is the "zero factor". Returns a
// T x (K + 1) matrix [zero factor, F_1, ..., F_K], or T x K when
// `with_zero_factor` is false (needed for sector dummies, which already sum
// to one).
std::optional<Matrix> EstimateCrossSectional(const Matrix& returns,
                                             const Matrix& attributes,
                                             bool with_zero_factor = true);

// 10.4.3 Factor-analytic approach via principal components of the sample
// covariance matrix. Loadings are eigenvectors scaled by sqrt(eigenvalue),
// so each factor has unit variance.
struct PrincipalFactors {
  Matrix loadings;            // N x K
  Vector eigenvalues;         // All N, decreasing.
  double explained_fraction;  // Share of total variance in the first K.
};
PrincipalFactors EstimatePrincipalComponents(const Matrix& returns,
                                             std::size_t num_factors);

// Sector-factor attributes (10.3.2): one dummy column per sector.
Matrix SectorDummies(std::span<const int> sector_of_security, int num_sectors);

}  // namespace investm::ch10

#endif  // CPP_CH10_FACTOR_MODELS_FACTOR_MODELS_H_
