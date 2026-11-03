#include "investm/formulas/fixed_income.h"

#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <span>

namespace investm::formulas {

double BreakEvenTaxRate(double tax_exempt_yield, double taxable_yield) {
  return 1.0 - tax_exempt_yield / taxable_yield;
}

double GeometricMeanGrowthRate(double beginning_value, double ending_value,
                               double years) {
  return std::pow(ending_value / beginning_value, 1.0 / years) - 1.0;
}

double RealReturn(double nominal_return, double inflation) {
  return (1.0 + nominal_return) / (1.0 + inflation) - 1.0;
}

double ApproxRealReturn(double nominal_return, double inflation) {
  return nominal_return - inflation;
}

double BondPrice(std::span<const double> coupons, double face_value,
                 double ytm) {
  assert(!coupons.empty());
  double price = 0.0;
  for (std::size_t t = 1; t <= coupons.size(); ++t) {
    price += coupons[t - 1] / std::pow(1.0 + ytm, static_cast<double>(t));
  }
  return price +
         face_value / std::pow(1.0 + ytm, static_cast<double>(coupons.size()));
}

double YieldToMaturity(double price, std::span<const double> coupons,
                       double face_value, double low, double high,
                       double tolerance) {
  double f_low = BondPrice(coupons, face_value, low) - price;
  const double f_high = BondPrice(coupons, face_value, high) - price;
  if (f_low * f_high > 0.0) return std::numeric_limits<double>::quiet_NaN();
  for (int i = 0; i < 200 && high - low >= tolerance; ++i) {
    const double mid = 0.5 * (low + high);
    const double f_mid = BondPrice(coupons, face_value, mid) - price;
    if (f_mid == 0.0) return mid;
    if ((f_mid > 0.0) == (f_low > 0.0)) {
      low = mid;
      f_low = f_mid;
    } else {
      high = mid;
    }
  }
  return 0.5 * (low + high);
}

double DiscountFactor(double spot_rate, double years) {
  return 1.0 / std::pow(1.0 + spot_rate, years);
}

double ForwardRate(double spot_prev, double spot_t, int t) {
  return std::pow(1.0 + spot_t, t) / std::pow(1.0 + spot_prev, t - 1) - 1.0;
}

double MacaulayDuration(std::span<const double> cash_flows, double ytm) {
  double price = 0.0;
  double weighted = 0.0;
  for (std::size_t t = 1; t <= cash_flows.size(); ++t) {
    const double pv =
        cash_flows[t - 1] / std::pow(1.0 + ytm, static_cast<double>(t));
    price += pv;
    weighted += pv * static_cast<double>(t);
  }
  return weighted / price;
}

double ModifiedDuration(double duration, double ytm) {
  return duration / (1.0 + ytm);
}

double EffectiveDuration(double price_yield_down, double price_yield_up,
                         double price, double yield_change) {
  return (price_yield_down - price_yield_up) / (2.0 * price * yield_change);
}

double PriceChangeFromDuration(double duration, double ytm,
                               double yield_change) {
  return -duration * yield_change / (1.0 + ytm);
}

double PriceChangeFromModifiedDuration(double modified_duration,
                                       double yield_change) {
  return -modified_duration * yield_change;
}

}  // namespace investm::formulas
