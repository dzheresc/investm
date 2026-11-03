#include "cpp/core/stats.h"

#include <cmath>
#include <numbers>
#include <numeric>

#include "cpp/core/check.h"

namespace investm::stats {
namespace {

void CheckSameSize(std::span<const double> x, std::span<const double> y) {
  INVESTM_CHECK(x.size() == y.size(), "size mismatch");
}

}  // namespace

double Mean(std::span<const double> x) {
  INVESTM_CHECK(!x.empty(), "Mean of empty range");
  return std::accumulate(x.begin(), x.end(), 0.0) /
         static_cast<double>(x.size());
}

double Covariance(std::span<const double> x, std::span<const double> y,
                  int ddof) {
  CheckSameSize(x, y);
  const double n = static_cast<double>(x.size());
  INVESTM_CHECK(n - ddof > 0, "too few observations");
  const double mean_x = Mean(x);
  const double mean_y = Mean(y);
  double sum = 0.0;
  for (std::size_t i = 0; i < x.size(); ++i) {
    sum += (x[i] - mean_x) * (y[i] - mean_y);
  }
  return sum / (n - ddof);
}

double Variance(std::span<const double> x, int ddof) {
  return Covariance(x, x, ddof);
}

double StdDev(std::span<const double> x, int ddof) {
  return std::sqrt(Variance(x, ddof));
}

double Correlation(std::span<const double> x, std::span<const double> y) {
  return Covariance(x, y) / (StdDev(x) * StdDev(y));
}

double Expected(std::span<const double> outcomes,
                std::span<const double> probs) {
  CheckSameSize(outcomes, probs);
  double e = 0.0;
  for (std::size_t i = 0; i < outcomes.size(); ++i) {
    e += probs[i] * outcomes[i];
  }
  return e;
}

double ExpectedCovariance(std::span<const double> x, std::span<const double> y,
                          std::span<const double> probs) {
  CheckSameSize(x, probs);
  CheckSameSize(y, probs);
  const double mean_x = Expected(x, probs);
  const double mean_y = Expected(y, probs);
  double c = 0.0;
  for (std::size_t i = 0; i < x.size(); ++i) {
    c += probs[i] * (x[i] - mean_x) * (y[i] - mean_y);
  }
  return c;
}

double Autocorrelation(std::span<const double> x, std::size_t lag) {
  INVESTM_CHECK(lag < x.size(), "lag too large");
  const double m = Mean(x);
  double num = 0.0;
  double den = 0.0;
  for (std::size_t t = 0; t < x.size(); ++t) {
    den += (x[t] - m) * (x[t] - m);
    if (t >= lag) num += (x[t] - m) * (x[t - lag] - m);
  }
  return num / den;
}

double GeometricMeanReturn(std::span<const double> returns) {
  INVESTM_CHECK(!returns.empty(), "empty return series");
  double log_sum = 0.0;
  for (double r : returns) {
    if (r <= -1.0) return -1.0;  // Total loss in some period.
    log_sum += std::log1p(r);
  }
  return std::expm1(log_sum / static_cast<double>(returns.size()));
}

SimpleRegression Ols(std::span<const double> x, std::span<const double> y) {
  CheckSameSize(x, y);
  INVESTM_CHECK(x.size() >= 3, "OLS needs at least 3 points");
  SimpleRegression r;
  r.n = x.size();
  const double mean_x = Mean(x);
  const double mean_y = Mean(y);
  double sxx = 0.0;
  double sxy = 0.0;
  double syy = 0.0;
  for (std::size_t i = 0; i < x.size(); ++i) {
    const double dx = x[i] - mean_x;
    const double dy = y[i] - mean_y;
    sxx += dx * dx;
    sxy += dx * dy;
    syy += dy * dy;
  }
  INVESTM_CHECK(sxx > 0.0, "regressor has zero variance");
  r.beta = sxy / sxx;
  r.alpha = mean_y - r.beta * mean_x;
  double sse = 0.0;
  for (std::size_t i = 0; i < x.size(); ++i) {
    const double e = y[i] - r.Predict(x[i]);
    sse += e * e;
  }
  const double n = static_cast<double>(r.n);
  r.r_squared = syy == 0.0 ? 1.0 : 1.0 - sse / syy;
  r.residual_std_dev = std::sqrt(sse / (n - 2.0));
  r.beta_std_error = r.residual_std_dev / std::sqrt(sxx);
  r.alpha_std_error =
      r.residual_std_dev * std::sqrt(1.0 / n + mean_x * mean_x / sxx);
  return r;
}

double NormPdf(double x) {
  return std::exp(-0.5 * x * x) / std::sqrt(2.0 * std::numbers::pi);
}

double NormCdf(double x) { return 0.5 * std::erfc(-x / std::numbers::sqrt2); }

}  // namespace investm::stats
