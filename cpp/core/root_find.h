// One-dimensional root finding.
//
// Yields, internal rates of return and implied volatilities are all "find x
// such that price(x) equals the observed price", so these solvers are used
// throughout the companion. All solvers return std::nullopt instead of
// throwing when they cannot find a root.

#ifndef CPP_CORE_ROOT_FIND_H_
#define CPP_CORE_ROOT_FIND_H_

#include <cmath>
#include <optional>
#include <utility>

namespace investm {

struct RootOptions {
  double tolerance = 1e-12;  // Absolute tolerance on x.
  int max_iterations = 200;
};

// Bisection. Slow but robust; requires f(lo) and f(hi) to differ in sign.
template <typename F>
std::optional<double> Bisect(F&& f, double lo, double hi,
                             RootOptions options = {}) {
  double f_lo = f(lo);
  const double f_hi = f(hi);
  if (f_lo == 0.0) return lo;
  if (f_hi == 0.0) return hi;
  if ((f_lo > 0.0) == (f_hi > 0.0)) return std::nullopt;
  for (int i = 0; i < options.max_iterations && hi - lo > options.tolerance;
       ++i) {
    const double mid = 0.5 * (lo + hi);
    const double f_mid = f(mid);
    if (f_mid == 0.0) return mid;
    if ((f_mid > 0.0) == (f_lo > 0.0)) {
      lo = mid;
      f_lo = f_mid;
    } else {
      hi = mid;
    }
  }
  return 0.5 * (lo + hi);
}

// Brent's method: inverse quadratic interpolation and secant steps with a
// bisection fallback. The default solver in this code base.
template <typename F>
std::optional<double> Brent(F&& f, double a, double b,
                            RootOptions options = {}) {
  double f_a = f(a);
  double f_b = f(b);
  if (f_a == 0.0) return a;
  if (f_b == 0.0) return b;
  if ((f_a > 0.0) == (f_b > 0.0)) return std::nullopt;
  if (std::fabs(f_a) < std::fabs(f_b)) {
    std::swap(a, b);
    std::swap(f_a, f_b);
  }
  double c = a;
  double f_c = f_a;
  double d = b - a;
  bool bisected = true;
  const double tol = options.tolerance;
  for (int i = 0; i < options.max_iterations; ++i) {
    if (f_b == 0.0 || std::fabs(b - a) < tol) return b;
    double s;
    if (f_a != f_c && f_b != f_c) {
      s = a * f_b * f_c / ((f_a - f_b) * (f_a - f_c)) +
          b * f_a * f_c / ((f_b - f_a) * (f_b - f_c)) +
          c * f_a * f_b / ((f_c - f_a) * (f_c - f_b));
    } else {
      s = b - f_b * (b - a) / (f_b - f_a);
    }
    const double quarter = (3.0 * a + b) / 4.0;
    const bool outside =
        !(s > std::fmin(quarter, b) && s < std::fmax(quarter, b));
    const bool slow = bisected ? std::fabs(s - b) >= std::fabs(b - c) / 2.0
                               : std::fabs(s - b) >= std::fabs(c - d) / 2.0;
    const bool tiny =
        bisected ? std::fabs(b - c) < tol : std::fabs(c - d) < tol;
    if (outside || slow || tiny) {
      s = 0.5 * (a + b);
      bisected = true;
    } else {
      bisected = false;
    }
    const double f_s = f(s);
    d = c;
    c = b;
    f_c = f_b;
    if ((f_a > 0.0) != (f_s > 0.0)) {
      b = s;
      f_b = f_s;
    } else {
      a = s;
      f_a = f_s;
    }
    if (std::fabs(f_a) < std::fabs(f_b)) {
      std::swap(a, b);
      std::swap(f_a, f_b);
    }
  }
  return b;
}

// Newton-Raphson with an analytic derivative. Fast when the starting point is
// good (e.g. implied volatility with vega).
template <typename F, typename DF>
std::optional<double> Newton(F&& f, DF&& df, double x0,
                             RootOptions options = {}) {
  double x = x0;
  for (int i = 0; i < options.max_iterations; ++i) {
    const double slope = df(x);
    if (slope == 0.0 || !std::isfinite(slope)) return std::nullopt;
    const double step = f(x) / slope;
    x -= step;
    if (std::fabs(step) < options.tolerance) return x;
  }
  return std::nullopt;
}

// Doubles the width of [lo, hi] upward until f changes sign, then runs Brent.
// Useful for yields, where no upper bound is known in advance.
template <typename F>
std::optional<double> BrentExpandUp(F&& f, double lo, double hi,
                                    RootOptions options = {}) {
  const double f_lo = f(lo);
  for (int i = 0; i < 60; ++i) {
    const double f_hi = f(hi);
    if (f_hi == 0.0 || (f_lo > 0.0) != (f_hi > 0.0)) {
      return Brent(f, lo, hi, options);
    }
    hi = lo + 2.0 * (hi - lo);
  }
  return std::nullopt;
}

}  // namespace investm

#endif  // CPP_CORE_ROOT_FIND_H_
