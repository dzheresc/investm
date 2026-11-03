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
  // Wichura (1988), Algorithm AS241 (PPND16): rational approximations
  // accurate to about 1e-16, so no refinement against the CDF is needed.
  // The Python version uses the same algorithm (statistics.NormalDist).
  const double q = p - 0.5;
  if (std::fabs(q) <= 0.425) {
    const double r = 0.180625 - q * q;
    const double num =
        (((((((2.5090809287301226727e+3 * r + 3.3430575583588128105e+4) * r +
              6.7265770927008700853e+4) *
                 r +
             4.5921953931549871457e+4) *
                r +
            1.3731693765509461125e+4) *
               r +
           1.9715909503065514427e+3) *
              r +
          1.3314166789178437745e+2) *
             r +
         3.3871328727963666080e+0) *
        q;
    const double den =
        (((((((5.2264952788528545610e+3 * r + 2.8729085735721942674e+4) * r +
              3.9307895800092710610e+4) *
                 r +
             2.1213794301586595867e+4) *
                r +
            5.3941960214247511077e+3) *
               r +
           6.8718700749205790830e+2) *
              r +
          4.2313330701600911252e+1) *
             r +
         1.0);
    return num / den;
  }
  double r = std::sqrt(-std::log(q < 0.0 ? p : 1.0 - p));
  double num;
  double den;
  if (r <= 5.0) {
    r -= 1.6;
    num = (((((((7.7454501427834140764e-4 * r + 2.2723844989269184583e-2) * r +
                2.4178072517745061177e-1) *
                   r +
               1.2704582524523683826e+0) *
                  r +
              3.6478483247632045605e+0) *
                 r +
             5.7694972214606914055e+0) *
                r +
            4.6303378461565452959e+0) *
               r +
           1.4234371107496835773e+0);
    den = (((((((1.0507500716444168432e-9 * r + 5.4759380849953449460e-4) * r +
                1.5198666563616457197e-2) *
                   r +
               1.4810397642748007459e-1) *
                  r +
              6.8976733498510000455e-1) *
                 r +
             1.6763848301838038494e+0) *
                r +
            2.0531916266377588219e+0) *
               r +
           1.0);
  } else {
    r -= 5.0;
    num = (((((((2.0103343992922881326e-7 * r + 2.7115555687434875782e-5) * r +
                1.2426609473880784386e-3) *
                   r +
               2.6532189526576123093e-2) *
                  r +
              2.9656057182850489123e-1) *
                 r +
             1.7848265399172913358e+0) *
                r +
            5.4637849111641143699e+0) *
               r +
           6.6579046435011037772e+0);
    den = (((((((2.0442631033899397856e-15 * r + 1.4215117583164458887e-7) * r +
                1.8463183175100546818e-5) *
                   r +
               7.8686913114561325910e-4) *
                  r +
              1.4875361290850614853e-2) *
                 r +
             1.3692988092273580531e-1) *
                r +
            5.9983220655588793769e-1) *
               r +
           1.0);
  }
  const double x = num / den;
  return q < 0.0 ? -x : x;
}

double NormalVaR(double mean, double std_dev, double alpha) {
  return -(mean + std_dev * NormInv(1.0 - alpha));
}

double NormalExpectedShortfall(double mean, double std_dev, double alpha) {
  const double z = NormInv(1.0 - alpha);
  return -mean + std_dev * stats::NormPdf(z) / (1.0 - alpha);
}

}  // namespace investm::ch06
