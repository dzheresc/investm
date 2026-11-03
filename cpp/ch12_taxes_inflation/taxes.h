// Chapter 12, Taxes and Inflation.
// Section 12.1: taxes in the United States. Progressive rate schedules,
// marginal versus average rates, after-tax returns, the taxable-equivalent
// yield of a tax-exempt bond, and the value of deferring tax.

#ifndef CPP_CH12_TAXES_INFLATION_TAXES_H_
#define CPP_CH12_TAXES_INFLATION_TAXES_H_

#include <vector>

namespace investm::ch12 {

// One bracket of a progressive schedule: income above `threshold` (up to the
// next bracket's threshold) is taxed at `rate`.
struct TaxBracket {
  double threshold;
  double rate;
};

// A progressive schedule. Brackets must start at a zero threshold and be
// strictly increasing in threshold.
class TaxSchedule {
 public:
  explicit TaxSchedule(std::vector<TaxBracket> brackets);

  // Total tax owed on `income`.
  double Tax(double income) const;
  // Rate applied to the next dollar of income.
  double MarginalRate(double income) const;
  // Tax / income (0 for zero income).
  double AverageRate(double income) const;

 private:
  std::vector<TaxBracket> brackets_;
};

// 12.1.1 After-tax return when the whole return is taxed at `tax_rate`.
inline double AfterTaxReturn(double pretax_return, double tax_rate) {
  return pretax_return * (1.0 - tax_rate);
}

// Combined marginal rate of a federal and a state income tax. When the state
// tax is deductible from federal taxable income the effective combined rate
// is t_f + t_s (1 - t_f).
double CombinedTaxRate(double federal, double state, bool state_deductible);

// 12.1.3 Yield a fully taxable bond must offer to match a tax-exempt yield.
double TaxableEquivalentYield(double tax_exempt_yield, double tax_rate);
// Tax rate at which a taxable and a tax-exempt bond are equally attractive.
double BreakevenTaxRate(double taxable_yield, double tax_exempt_yield);

// 12.1.2 Tax deferral. Terminal wealth of `amount` invested for `years` at
// pretax rate `r` when
//   - gains are taxed every year at `tax_rate` (e.g. interest income),
//   - gains are taxed once, when realized at the end (e.g. unrealized
//     capital gains, tax-deferred accounts),
//   - gains are never taxed.
double WealthTaxedAnnually(double amount, double r, double tax_rate,
                           double years);
double WealthTaxedAtEnd(double amount, double r, double tax_rate, double years);
double WealthUntaxed(double amount, double r, double years);
// Annual after-tax rate equivalent to deferral: the rate g with
// amount (1+g)^years = WealthTaxedAtEnd(...).
double EffectiveDeferredRate(double r, double tax_rate, double years);

}  // namespace investm::ch12

#endif  // CPP_CH12_TAXES_INFLATION_TAXES_H_
