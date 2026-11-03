#include "cpp/ch12_taxes_inflation/taxes.h"

#include <algorithm>
#include <cmath>
#include <utility>

#include "cpp/core/check.h"

namespace investm::ch12 {

TaxSchedule::TaxSchedule(std::vector<TaxBracket> brackets)
    : brackets_(std::move(brackets)) {
  INVESTM_CHECK(!brackets_.empty(), "schedule needs at least one bracket");
  INVESTM_CHECK(brackets_.front().threshold == 0.0,
                "first bracket must start at zero");
  for (std::size_t i = 1; i < brackets_.size(); ++i) {
    INVESTM_CHECK(brackets_[i].threshold > brackets_[i - 1].threshold,
                  "thresholds must increase");
  }
}

double TaxSchedule::Tax(double income) const {
  INVESTM_CHECK(income >= 0.0, "income must be non-negative");
  double tax = 0.0;
  for (std::size_t i = 0; i < brackets_.size(); ++i) {
    const double lo = brackets_[i].threshold;
    if (income <= lo) break;
    const double hi =
        i + 1 < brackets_.size() ? brackets_[i + 1].threshold : income;
    tax += (std::min(income, hi) - lo) * brackets_[i].rate;
  }
  return tax;
}

double TaxSchedule::MarginalRate(double income) const {
  INVESTM_CHECK(income >= 0.0, "income must be non-negative");
  double rate = brackets_.front().rate;
  for (const TaxBracket& b : brackets_) {
    if (income >= b.threshold) rate = b.rate;
  }
  return rate;
}

double TaxSchedule::AverageRate(double income) const {
  return income > 0.0 ? Tax(income) / income : 0.0;
}

double CombinedTaxRate(double federal, double state, bool state_deductible) {
  INVESTM_CHECK(federal >= 0.0 && federal < 1.0, "federal rate in [0, 1)");
  INVESTM_CHECK(state >= 0.0 && state < 1.0, "state rate in [0, 1)");
  return state_deductible ? federal + state * (1.0 - federal) : federal + state;
}

double TaxableEquivalentYield(double tax_exempt_yield, double tax_rate) {
  INVESTM_CHECK(tax_rate >= 0.0 && tax_rate < 1.0, "tax rate in [0, 1)");
  return tax_exempt_yield / (1.0 - tax_rate);
}

double BreakevenTaxRate(double taxable_yield, double tax_exempt_yield) {
  INVESTM_CHECK(taxable_yield > 0.0, "taxable yield must be positive");
  return 1.0 - tax_exempt_yield / taxable_yield;
}

double WealthTaxedAnnually(double amount, double r, double tax_rate,
                           double years) {
  return amount * std::pow(1.0 + r * (1.0 - tax_rate), years);
}

double WealthTaxedAtEnd(double amount, double r, double tax_rate,
                        double years) {
  const double gross = amount * std::pow(1.0 + r, years);
  return gross - tax_rate * (gross - amount);
}

double WealthUntaxed(double amount, double r, double years) {
  return amount * std::pow(1.0 + r, years);
}

double EffectiveDeferredRate(double r, double tax_rate, double years) {
  INVESTM_CHECK(years > 0.0, "horizon must be positive");
  return std::pow(WealthTaxedAtEnd(1.0, r, tax_rate, years), 1.0 / years) - 1.0;
}

}  // namespace investm::ch12
