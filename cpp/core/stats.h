// Descriptive statistics and simple regression.
//
// Used by Ch. 1 (risk and return), Ch. 4 (event studies, random-walk tests),
// Ch. 6 Appendix A (moments of distributions), Ch. 7.3 and 9.4 (the market
// model), Ch. 10 (factor models) and Ch. 24 (performance evaluation).

#ifndef CPP_CORE_STATS_H_
#define CPP_CORE_STATS_H_

#include <cstddef>
#include <span>

namespace investm::stats {

double Mean(std::span<const double> x);

// `ddof` is the delta degrees of freedom: 1 gives the unbiased sample
// estimator, 0 the population formula.
double Covariance(std::span<const double> x, std::span<const double> y,
                  int ddof = 1);
double Variance(std::span<const double> x, int ddof = 1);
double StdDev(std::span<const double> x, int ddof = 1);
double Correlation(std::span<const double> x, std::span<const double> y);

// Probability-weighted moments of a discrete distribution (scenario
// analysis). `probs` must have the same length as the outcomes.
double Expected(std::span<const double> outcomes,
                std::span<const double> probs);
double ExpectedCovariance(std::span<const double> x, std::span<const double> y,
                          std::span<const double> probs);

// Lag-k sample autocorrelation.
double Autocorrelation(std::span<const double> x, std::size_t lag);

// Compound per-period return: (prod (1 + r_t))^(1/T) - 1.
double GeometricMeanReturn(std::span<const double> returns);

// Ordinary least squares fit of y = alpha + beta * x + e.
struct SimpleRegression {
  double alpha = 0.0;
  double beta = 0.0;
  double r_squared = 0.0;
  double residual_std_dev = 0.0;  // Standard error of the regression.
  double alpha_std_error = 0.0;
  double beta_std_error = 0.0;
  std::size_t n = 0;

  double Predict(double x) const { return alpha + beta * x; }
};

SimpleRegression Ols(std::span<const double> x, std::span<const double> y);

// Standard normal density and distribution function.
double NormPdf(double x);
double NormCdf(double x);

}  // namespace investm::stats

#endif  // CPP_CORE_STATS_H_
