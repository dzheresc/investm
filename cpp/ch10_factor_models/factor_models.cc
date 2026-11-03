#include "cpp/ch10_factor_models/factor_models.h"

#include <algorithm>
#include <cmath>

#include "cpp/core/check.h"
#include "cpp/core/regression.h"

namespace investm::ch10 {

Vector FactorExpectedReturns(const FactorModel& model,
                             std::span<const double> factor_means) {
  Vector mu = model.sensitivities * factor_means;
  for (std::size_t i = 0; i < mu.size(); ++i) mu[i] += model.intercepts[i];
  return mu;
}

Matrix FactorCovariance(const FactorModel& model, const Matrix& factor_cov) {
  INVESTM_CHECK(factor_cov.rows() == model.num_factors(),
                "factor covariance has the wrong size");
  Matrix cov =
      model.sensitivities * factor_cov * model.sensitivities.Transpose();
  for (std::size_t i = 0; i < cov.rows(); ++i) {
    cov(i, i) += model.residual_variances[i];
  }
  return cov;
}

Vector PortfolioSensitivities(const FactorModel& model,
                              std::span<const double> weights) {
  return model.sensitivities.Transpose() * weights;
}

VarianceDecomposition DecomposePortfolioVariance(
    const FactorModel& model, const Matrix& factor_cov,
    std::span<const double> weights) {
  const Vector b = PortfolioSensitivities(model, weights);
  double residual = 0.0;
  for (std::size_t i = 0; i < weights.size(); ++i) {
    residual += weights[i] * weights[i] * model.residual_variances[i];
  }
  return {QuadraticForm(b, factor_cov), residual};
}

std::optional<FactorModel> EstimateTimeSeries(const Matrix& returns,
                                              const Matrix& factors) {
  INVESTM_CHECK(returns.rows() == factors.rows(),
                "returns and factors cover different periods");
  const std::size_t n = returns.cols();
  const std::size_t k = factors.cols();
  // Every security is regressed on the same factors, so (X'X)^-1 is
  // computed once rather than once per security.
  const auto design = OlsDesign::Create(factors);
  if (!design.has_value()) return std::nullopt;
  FactorModel model{Vector(n), Matrix(n, k), Vector(n)};
  for (std::size_t i = 0; i < n; ++i) {
    const LinearRegression fit = design->Fit(returns.Column(i));
    model.intercepts[i] = fit.coefficients[0];
    for (std::size_t j = 0; j < k; ++j) {
      model.sensitivities(i, j) = fit.coefficients[j + 1];
    }
    model.residual_variances[i] = fit.residual_std_dev * fit.residual_std_dev;
  }
  return model;
}

std::optional<Matrix> EstimateCrossSectional(const Matrix& returns,
                                             const Matrix& attributes,
                                             bool with_zero_factor) {
  INVESTM_CHECK(returns.cols() == attributes.rows(),
                "need one attribute row per security");
  const std::size_t width = attributes.cols() + (with_zero_factor ? 1 : 0);
  Matrix factor_values(returns.rows(), width);
  for (std::size_t t = 0; t < returns.rows(); ++t) {
    const auto fit = MultipleOls(attributes, returns.Row(t), with_zero_factor);
    if (!fit.has_value()) return std::nullopt;
    for (std::size_t j = 0; j < width; ++j) {
      factor_values(t, j) = fit->coefficients[j];
    }
  }
  return factor_values;
}

PrincipalFactors EstimatePrincipalComponents(const Matrix& returns,
                                             std::size_t num_factors) {
  const Matrix cov = CovarianceMatrix(returns);
  INVESTM_CHECK(num_factors >= 1 && num_factors <= cov.rows(),
                "invalid number of factors");
  const SymmetricEigen eig = EigenSymmetric(cov);
  PrincipalFactors pf{Matrix(cov.rows(), num_factors), eig.values, 0.0};
  double total = 0.0;
  double kept = 0.0;
  for (std::size_t k = 0; k < eig.values.size(); ++k) {
    total += eig.values[k];
    if (k < num_factors) kept += eig.values[k];
  }
  for (std::size_t k = 0; k < num_factors; ++k) {
    // Sign convention: make the loadings sum to a non-negative number.
    double sum = 0.0;
    for (std::size_t i = 0; i < cov.rows(); ++i) sum += eig.vectors(i, k);
    const double sign = sum < 0.0 ? -1.0 : 1.0;
    const double scale = sign * std::sqrt(std::max(eig.values[k], 0.0));
    for (std::size_t i = 0; i < cov.rows(); ++i) {
      pf.loadings(i, k) = scale * eig.vectors(i, k);
    }
  }
  pf.explained_fraction = kept / total;
  return pf;
}

Matrix SectorDummies(std::span<const int> sector_of_security, int num_sectors) {
  INVESTM_CHECK(num_sectors >= 1, "need at least one sector");
  Matrix d(sector_of_security.size(), static_cast<std::size_t>(num_sectors));
  for (std::size_t i = 0; i < sector_of_security.size(); ++i) {
    const int s = sector_of_security[i];
    INVESTM_CHECK(s >= 0 && s < num_sectors, "sector out of range");
    d(i, static_cast<std::size_t>(s)) = 1.0;
  }
  return d;
}

}  // namespace investm::ch10
