#include "cpp/ch12_taxes_inflation/inflation.h"

#include <algorithm>
#include <vector>

#include "cpp/core/check.h"

namespace investm::ch12 {

std::vector<double> InflationRates(std::span<const double> index_levels) {
  INVESTM_CHECK(index_levels.size() >= 2, "need at least two index levels");
  std::vector<double> rates;
  rates.reserve(index_levels.size() - 1);
  for (std::size_t t = 1; t < index_levels.size(); ++t) {
    INVESTM_CHECK(index_levels[t - 1] > 0.0, "index levels must be positive");
    rates.push_back(index_levels[t] / index_levels[t - 1] - 1.0);
  }
  return rates;
}

double CumulativeInflation(std::span<const double> index_levels) {
  INVESTM_CHECK(index_levels.size() >= 2, "need at least two index levels");
  INVESTM_CHECK(index_levels.front() > 0.0, "index levels must be positive");
  return index_levels.back() / index_levels.front() - 1.0;
}

double PurchasingPower(std::span<const double> index_levels) {
  return 1.0 / (1.0 + CumulativeInflation(index_levels));
}

std::vector<double> RealReturns(std::span<const double> nominal,
                                std::span<const double> inflation) {
  INVESTM_CHECK(nominal.size() == inflation.size(), "size mismatch");
  std::vector<double> real(nominal.size());
  for (std::size_t t = 0; t < nominal.size(); ++t) {
    real[t] = (1.0 + nominal[t]) / (1.0 + inflation[t]) - 1.0;
  }
  return real;
}

double FisherNominalRate(double real, double expected_inflation) {
  return (1.0 + real) * (1.0 + expected_inflation) - 1.0;
}

double RealAfterTaxReturn(double nominal, double tax_rate, double inflation) {
  return (1.0 + nominal * (1.0 - tax_rate)) / (1.0 + inflation) - 1.0;
}

double EffectiveTaxOnRealReturn(double nominal, double tax_rate,
                                double inflation) {
  const double real = (1.0 + nominal) / (1.0 + inflation) - 1.0;
  INVESTM_CHECK(real > 0.0, "pretax real return must be positive");
  return 1.0 - RealAfterTaxReturn(nominal, tax_rate, inflation) / real;
}

ch05::CashFlows IndexedBondCashFlows(double face, double real_coupon,
                                     double base_index,
                                     std::span<const double> index_path,
                                     int frequency) {
  INVESTM_CHECK(frequency > 0, "frequency must be positive");
  INVESTM_CHECK(base_index > 0.0, "base index must be positive");
  INVESTM_CHECK(!index_path.empty(), "need at least one coupon date");
  ch05::CashFlows flows;
  flows.reserve(index_path.size());
  for (std::size_t k = 0; k < index_path.size(); ++k) {
    const double principal = face * index_path[k] / base_index;
    double amount = principal * real_coupon / frequency;
    if (k + 1 == index_path.size()) amount += std::max(principal, face);
    flows.push_back({static_cast<double>(k + 1) / frequency, amount});
  }
  return flows;
}

double BreakevenInflation(double nominal_yield, double real_yield) {
  return (1.0 + nominal_yield) / (1.0 + real_yield) - 1.0;
}

stats::SimpleRegression InflationBeta(std::span<const double> nominal_returns,
                                      std::span<const double> inflation) {
  return stats::Ols(inflation, nominal_returns);
}

}  // namespace investm::ch12
