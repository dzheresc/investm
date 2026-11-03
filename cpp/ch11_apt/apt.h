// Chapter 11, Arbitrage Pricing Theory.
// Sections 11.1 (factor models, the principle of arbitrage, arbitrage
// portfolios), 11.2 (pricing effects), 11.3-11.4 (the APT pricing equation
// and its multiple-factor form), 11.5 (synthesis of the APT and CAPM) and
// 11.6 (identifying and estimating the factors).
//
// If returns follow r = a + B F + e and well-diversified portfolios cannot
// earn riskless profits, expected returns must satisfy
//   E[r_i] = lambda_0 + lambda_1 b_i1 + ... + lambda_K b_iK.

#ifndef CPP_CH11_APT_APT_H_
#define CPP_CH11_APT_APT_H_

#include <optional>
#include <span>

#include "cpp/core/matrix.h"

namespace investm::ch11 {

// 11.1.2 An arbitrage portfolio changes holdings by X with
//   sum X = 0 (no new money), B'X = 0 (no factor risk), mu'X > 0.
bool IsArbitragePortfolio(std::span<const double> x, const Matrix& b,
                          std::span<const double> mu, double tolerance = 1e-9);

// Finds an arbitrage portfolio if one exists: the component of mu orthogonal
// to [1, B], scaled so that total purchases equal `scale` (sum of positive
// X). Returns nullopt when mu already lies on the APT plane.
std::optional<Vector> FindArbitragePortfolio(std::span<const double> mu,
                                             const Matrix& b,
                                             double scale = 1.0,
                                             double tolerance = 1e-10);

// 11.4 Fits the APT plane by cross-sectional regression of expected returns
// on sensitivities. Exact when no arbitrage is possible.
struct AptPricing {
  double lambda0;  // Return on a zero-sensitivity (riskfree) portfolio.
  Vector lambdas;  // Factor risk premiums.
  Vector pricing_errors;
};
std::optional<AptPricing> FitAptPricing(std::span<const double> mu,
                                        const Matrix& b);

double AptExpectedReturn(double lambda0, std::span<const double> lambdas,
                         std::span<const double> sensitivities);

// A "pure factor" portfolio: fully invested, sensitivity 1 to factor k and 0
// to the others (minimum-norm weights). Under the APT its expected return is
// lambda_0 + lambda_k.
std::optional<Vector> PureFactorPortfolio(const Matrix& b, std::size_t k);

}  // namespace investm::ch11

#endif  // CPP_CH11_APT_APT_H_
