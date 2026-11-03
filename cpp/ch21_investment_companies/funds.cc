#include "cpp/ch21_investment_companies/funds.h"

#include <cmath>

#include "cpp/core/check.h"

namespace investm::ch21 {

double NetAssetValue(double assets, double liabilities, double shares) {
  INVESTM_CHECK(shares > 0.0, "shares must be positive");
  return (assets - liabilities) / shares;
}

double PremiumOrDiscount(double price, double nav) {
  INVESTM_CHECK(nav > 0.0, "NAV must be positive");
  return price / nav - 1.0;
}

double OfferingPrice(double nav, double front_load) {
  INVESTM_CHECK(front_load >= 0.0 && front_load < 1.0, "load in [0, 1)");
  return nav / (1.0 - front_load);
}

double LoadAsFractionOfInvestment(double front_load) {
  INVESTM_CHECK(front_load >= 0.0 && front_load < 1.0, "load in [0, 1)");
  return front_load / (1.0 - front_load);
}

double FundReturn(double nav0, double nav1, double income_distribution,
                  double capital_gain_distribution) {
  INVESTM_CHECK(nav0 > 0.0, "NAV must be positive");
  return (nav1 - nav0 + income_distribution + capital_gain_distribution) / nav0;
}

double SharesAfterReinvestment(double shares, double distribution_per_share,
                               double reinvest_nav) {
  INVESTM_CHECK(reinvest_nav > 0.0, "NAV must be positive");
  return shares * (1.0 + distribution_per_share / reinvest_nav);
}

double TerminalValue(double amount, double gross_return, double expense_ratio,
                     int years, double front_load,
                     std::span<const double> back_loads) {
  INVESTM_CHECK(years >= 0, "years must be non-negative");
  INVESTM_CHECK(expense_ratio >= 0.0 && expense_ratio < 1.0,
                "expense ratio in [0, 1)");
  INVESTM_CHECK(front_load >= 0.0 && front_load < 1.0, "load in [0, 1)");
  double value = amount * (1.0 - front_load);
  value *= std::pow((1.0 + gross_return) * (1.0 - expense_ratio), years);
  const auto y = static_cast<std::size_t>(years);
  if (y >= 1 && y <= back_loads.size()) value *= 1.0 - back_loads[y - 1];
  return value;
}

double AnnualizedFeeDrag(double gross_return, double expense_ratio, int years,
                         double front_load,
                         std::span<const double> back_loads) {
  INVESTM_CHECK(years > 0, "years must be positive");
  const double tv = TerminalValue(1.0, gross_return, expense_ratio, years,
                                  front_load, back_loads);
  return gross_return - (std::pow(tv, 1.0 / years) - 1.0);
}

std::optional<int> BreakevenYears(const FundTerms& a, const FundTerms& b,
                                  double gross_return, int max_years) {
  for (int y = 1; y <= max_years; ++y) {
    if (TerminalValue(1.0, gross_return, a.expense_ratio, y, a.front_load) >=
        TerminalValue(1.0, gross_return, b.expense_ratio, y, b.front_load)) {
      return y;
    }
  }
  return std::nullopt;
}

}  // namespace investm::ch21
