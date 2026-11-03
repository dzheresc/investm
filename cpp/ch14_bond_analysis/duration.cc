#include "cpp/ch14_bond_analysis/duration.h"

#include <cmath>

#include "cpp/core/check.h"

namespace investm::ch14 {
namespace {

struct Moments {
  double price = 0.0;
  double time_weighted = 0.0;  // sum t PV_t
  double convexity_sum = 0.0;  // sum t (t + 1/f) PV_t
};

Moments Compute(const CashFlows& flows, double yield, int frequency) {
  INVESTM_CHECK(frequency > 0, "frequency must be positive");
  const double g = 1.0 + yield / frequency;
  INVESTM_CHECK(g > 0.0, "yield below -100% per period");
  Moments m;
  for (const auto& f : flows) {
    const double pv = f.amount / std::pow(g, f.time * frequency);
    m.price += pv;
    m.time_weighted += f.time * pv;
    m.convexity_sum += f.time * (f.time + 1.0 / frequency) * pv;
  }
  INVESTM_CHECK(m.price > 0.0, "bond price must be positive");
  return m;
}

}  // namespace

double MacaulayDuration(const CashFlows& flows, double yield, int frequency) {
  const Moments m = Compute(flows, yield, frequency);
  return m.time_weighted / m.price;
}

double ModifiedDuration(const CashFlows& flows, double yield, int frequency) {
  return MacaulayDuration(flows, yield, frequency) / (1.0 + yield / frequency);
}

double DollarDuration(const CashFlows& flows, double yield, int frequency) {
  const Moments m = Compute(flows, yield, frequency);
  return m.time_weighted / (1.0 + yield / frequency);
}

double PriceValueOfBasisPoint(const CashFlows& flows, double yield,
                              int frequency) {
  return DollarDuration(flows, yield, frequency) * 1e-4;
}

double Convexity(const CashFlows& flows, double yield, int frequency) {
  const Moments m = Compute(flows, yield, frequency);
  const double g = 1.0 + yield / frequency;
  return m.convexity_sum / (m.price * g * g);
}

double ApproxPriceChange(double price, double modified_duration,
                         double convexity, double dy) {
  return price * (-modified_duration * dy + 0.5 * convexity * dy * dy);
}

double PerpetuityDuration(double yield) {
  INVESTM_CHECK(yield > 0.0, "yield must be positive");
  return (1.0 + yield) / yield;
}

double EffectiveDuration(const Pricer& price, double yield, double dy) {
  INVESTM_CHECK(dy > 0.0, "bump must be positive");
  const double p0 = price(yield);
  return (price(yield - dy) - price(yield + dy)) / (2.0 * p0 * dy);
}

double EffectiveConvexity(const Pricer& price, double yield, double dy) {
  INVESTM_CHECK(dy > 0.0, "bump must be positive");
  const double p0 = price(yield);
  return (price(yield - dy) + price(yield + dy) - 2.0 * p0) / (p0 * dy * dy);
}

}  // namespace investm::ch14
