#include "cpp/ch06_portfolio_selection/risk_measures.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "cpp/core/check.h"
#include "cpp/core/stats.h"

namespace investm::ch06 {

double SemiVariance(std::span<const double> returns, double target) {
  INVESTM_CHECK(!returns.empty(), "empty return series");
  double s = 0.0;
  for (double r : returns) {
    if (r < target) s += (r - target) * (r - target);
  }
  return s / static_cast<double>(returns.size());
}

double DownsideDeviation(std::span<const double> returns, double target) {
  return std::sqrt(SemiVariance(returns, target));
}

double ShortfallProbability(std::span<const double> returns, double target) {
  INVESTM_CHECK(!returns.empty(), "empty return series");
  const auto n = std::count_if(returns.begin(), returns.end(),
                               [target](double r) { return r < target; });
  return static_cast<double>(n) / static_cast<double>(returns.size());
}

namespace {

// Number of observations in the (1 - alpha) tail, at least one.
std::size_t TailCount(std::size_t n, double alpha) {
  INVESTM_CHECK(alpha > 0.0 && alpha < 1.0, "alpha must be in (0, 1)");
  const auto k = static_cast<std::size_t>(
      std::ceil((1.0 - alpha) * static_cast<double>(n) - 1e-9));
  return std::max<std::size_t>(k, 1);
}

}  // namespace

double HistoricalVaR(std::span<const double> returns, double alpha) {
  INVESTM_CHECK(!returns.empty(), "empty return series");
  std::vector<double> sorted(returns.begin(), returns.end());
  const std::size_t k = TailCount(sorted.size(), alpha);
  std::nth_element(sorted.begin(),
                   sorted.begin() + static_cast<std::ptrdiff_t>(k - 1),
                   sorted.end());
  return -sorted[k - 1];
}

double HistoricalExpectedShortfall(std::span<const double> returns,
                                   double alpha) {
  INVESTM_CHECK(!returns.empty(), "empty return series");
  std::vector<double> sorted(returns.begin(), returns.end());
  std::sort(sorted.begin(), sorted.end());
  const std::size_t k = TailCount(sorted.size(), alpha);
  double s = 0.0;
  for (std::size_t i = 0; i < k; ++i) s += sorted[i];
  return -s / static_cast<double>(k);
}

double NormInv(double p) {
  INVESTM_CHECK(p > 0.0 && p < 1.0, "probability must be in (0, 1)");
  static constexpr double a[] = {-3.969683028665376e+01, 2.209460984245205e+02,
                                 -2.759285104469687e+02, 1.383577518672690e+02,
                                 -3.066479806614716e+01, 2.506628277459239e+00};
  static constexpr double b[] = {-5.447609879822406e+01, 1.615858368580409e+02,
                                 -1.556989798598866e+02, 6.680131188771972e+01,
                                 -1.328068155288572e+01};
  static constexpr double c[] = {-7.784894002430293e-03, -3.223964580411365e-01,
                                 -2.400758277161838e+00, -2.549732539343734e+00,
                                 4.374664141464968e+00,  2.938163982698783e+00};
  static constexpr double d[] = {7.784695709041462e-03, 3.224671290700398e-01,
                                 2.445134137142996e+00, 3.754408661907416e+00};
  constexpr double kLow = 0.02425;
  double x;
  if (p < kLow) {
    const double q = std::sqrt(-2.0 * std::log(p));
    x = (((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5]) /
        ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1.0);
  } else if (p <= 1.0 - kLow) {
    const double q = p - 0.5;
    const double r = q * q;
    x = (((((a[0] * r + a[1]) * r + a[2]) * r + a[3]) * r + a[4]) * r + a[5]) *
        q /
        (((((b[0] * r + b[1]) * r + b[2]) * r + b[3]) * r + b[4]) * r + 1.0);
  } else {
    const double q = std::sqrt(-2.0 * std::log(1.0 - p));
    x = -(((((c[0] * q + c[1]) * q + c[2]) * q + c[3]) * q + c[4]) * q + c[5]) /
        ((((d[0] * q + d[1]) * q + d[2]) * q + d[3]) * q + 1.0);
  }
  // One Newton step against the exact CDF.
  const double e = stats::NormCdf(x) - p;
  return x - e / stats::NormPdf(x);
}

double NormalVaR(double mean, double std_dev, double alpha) {
  return -(mean + std_dev * NormInv(1.0 - alpha));
}

double NormalExpectedShortfall(double mean, double std_dev, double alpha) {
  const double z = NormInv(1.0 - alpha);
  return -mean + std_dev * stats::NormPdf(z) / (1.0 - alpha);
}

}  // namespace investm::ch06
