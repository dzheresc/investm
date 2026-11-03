#include "investm/formulas/regression.h"

#include <cmath>
#include <cstddef>
#include <limits>
#include <optional>
#include <span>

namespace investm::formulas {
namespace {

constexpr double kNaN = std::numeric_limits<double>::quiet_NaN();

struct Sums {
  double t = 0.0;
  double x = 0.0;
  double y = 0.0;
  double xx = 0.0;
  double yy = 0.0;
  double xy = 0.0;
};

std::optional<Sums> ComputeSums(std::span<const double> x,
                                std::span<const double> y) {
  if (x.size() != y.size() || x.size() < 2) return std::nullopt;
  Sums s;
  s.t = static_cast<double>(x.size());
  for (std::size_t i = 0; i < x.size(); ++i) {
    s.x += x[i];
    s.y += y[i];
    s.xx += x[i] * x[i];
    s.yy += y[i] * y[i];
    s.xy += x[i] * y[i];
  }
  return s;
}

}  // namespace

double RegressionBeta(std::span<const double> x, std::span<const double> y) {
  const auto s = ComputeSums(x, y);
  if (!s) return kNaN;
  return (s->t * s->xy - s->y * s->x) / (s->t * s->xx - s->x * s->x);
}

double RegressionAlpha(std::span<const double> x, std::span<const double> y,
                       double beta) {
  const auto s = ComputeSums(x, y);
  if (!s) return kNaN;
  return s->y / s->t - beta * s->x / s->t;
}

double RegressionResidualStdDev(std::span<const double> x,
                                std::span<const double> y, double alpha,
                                double beta) {
  const auto s = ComputeSums(x, y);
  if (!s || s->t < 3.0) return kNaN;
  const double sse = s->yy - alpha * s->y - beta * s->xy;
  return std::sqrt((sse > 0.0 ? sse : 0.0) / (s->t - 2.0));
}

double RegressionBetaStdError(std::span<const double> x,
                              double residual_std_dev) {
  const auto s = ComputeSums(x, x);
  if (!s) return kNaN;
  return residual_std_dev / std::sqrt(s->xx - s->x * s->x / s->t);
}

double RegressionAlphaStdError(std::span<const double> x,
                               double residual_std_dev) {
  const auto s = ComputeSums(x, x);
  if (!s) return kNaN;
  return residual_std_dev / std::sqrt(s->t - s->x * s->x / s->xx);
}

double CorrelationCoefficient(std::span<const double> x,
                              std::span<const double> y) {
  const auto s = ComputeSums(x, y);
  if (!s) return kNaN;
  const double num = s->t * s->xy - s->y * s->x;
  const double den =
      std::sqrt((s->t * s->yy - s->y * s->y) * (s->t * s->xx - s->x * s->x));
  return num / den;
}

}  // namespace investm::formulas
