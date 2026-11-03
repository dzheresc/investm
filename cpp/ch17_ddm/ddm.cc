#include "cpp/ch17_ddm/ddm.h"

#include <cmath>
#include <vector>

#include "cpp/core/check.h"
#include "cpp/core/root_find.h"

namespace investm::ch17 {

double PresentValueOfDividends(std::span<const double> dividends, double k) {
  INVESTM_CHECK(k > -1.0, "discount rate must exceed -100%");
  double pv = 0.0;
  double factor = 1.0;
  for (double d : dividends) {
    factor /= 1.0 + k;
    pv += d * factor;
  }
  return pv;
}

double ZeroGrowthValue(double dividend, double k) {
  INVESTM_CHECK(k > 0.0, "required return must be positive");
  return dividend / k;
}

double ConstantGrowthValue(double d0, double g, double k) {
  INVESTM_CHECK(k > g, "required return must exceed growth");
  return d0 * (1.0 + g) / (k - g);
}

double ConstantGrowthImpliedReturn(double price, double d0, double g) {
  INVESTM_CHECK(price > 0.0, "price must be positive");
  return d0 * (1.0 + g) / price + g;
}

double MultipleGrowthValue(std::span<const double> explicit_dividends, double g,
                           double k) {
  INVESTM_CHECK(!explicit_dividends.empty(), "need explicit dividends");
  INVESTM_CHECK(k > g, "required return must exceed growth");
  const double t = static_cast<double>(explicit_dividends.size());
  const double terminal =
      explicit_dividends.back() * (1.0 + g) / (k - g) / std::pow(1.0 + k, t);
  return PresentValueOfDividends(explicit_dividends, k) + terminal;
}

std::optional<double> MultipleGrowthImpliedReturn(
    double price, std::span<const double> explicit_dividends, double g) {
  INVESTM_CHECK(price > 0.0, "price must be positive");
  // Value falls from +infinity (k just above g) toward 0 as k grows.
  auto f = [&](double k) {
    return MultipleGrowthValue(explicit_dividends, g, k) - price;
  };
  return BrentExpandUp(f, g + 1e-9, g + 0.5);
}

std::vector<double> ThreePhaseDividends(double d0, double g_a, int t1, int t2,
                                        double g_c) {
  INVESTM_CHECK(t1 >= 0 && t2 > t1, "need 0 <= t1 < t2");
  std::vector<double> d;
  d.reserve(static_cast<std::size_t>(t2));
  double prev = d0;
  for (int t = 1; t <= t2; ++t) {
    double g = g_a;
    if (t > t1) {
      g = g_a - (g_a - g_c) * static_cast<double>(t - t1) /
                    static_cast<double>(t2 - t1);
    }
    prev *= 1.0 + g;
    d.push_back(prev);
  }
  return d;
}

double ThreePhaseValue(double d0, double g_a, int t1, int t2, double g_c,
                       double k) {
  return MultipleGrowthValue(ThreePhaseDividends(d0, g_a, t1, t2, g_c), g_c, k);
}

double ForwardPriceEarnings(double payout, double g, double k) {
  INVESTM_CHECK(k > g, "required return must exceed growth");
  return payout / (k - g);
}

double TrailingPriceEarnings(double payout, double g, double k) {
  return ForwardPriceEarnings(payout, g, k) * (1.0 + g);
}

double SustainableGrowth(double roe, double retention) {
  INVESTM_CHECK(retention >= 0.0 && retention <= 1.0, "retention in [0, 1]");
  return roe * retention;
}

double ValueWithReinvestment(double e1, double retention, double roe,
                             double k) {
  const double g = SustainableGrowth(roe, retention);
  INVESTM_CHECK(k > g, "required return must exceed growth");
  return e1 * (1.0 - retention) / (k - g);
}

double PresentValueOfGrowthOpportunities(double price, double e1, double k) {
  INVESTM_CHECK(k > 0.0, "required return must be positive");
  return price - e1 / k;
}

}  // namespace investm::ch17
