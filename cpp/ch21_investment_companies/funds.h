// Chapter 21, Investment Companies.
// Net asset value, closed-end fund premiums and discounts, loads, expense
// ratios and their compounding cost, fund returns with distributions, and
// the holding period at which a load fund beats a no-load fund.

#ifndef CPP_CH21_INVESTMENT_COMPANIES_FUNDS_H_
#define CPP_CH21_INVESTMENT_COMPANIES_FUNDS_H_

#include <optional>
#include <span>

namespace investm::ch21 {

// 21.2 Net asset value per share.
double NetAssetValue(double assets, double liabilities, double shares);
// 21.3 Closed-end funds: price / NAV - 1 (negative = discount).
double PremiumOrDiscount(double price, double nav);

// 21.4 Open-end funds. Offering price with a front-end load quoted as a
// fraction of the offering price: NAV / (1 - load). The load as a fraction
// of the money actually invested is load / (1 - load).
double OfferingPrice(double nav, double front_load);
double LoadAsFractionOfInvestment(double front_load);

// One-period fund return with income and capital-gain distributions:
// (NAV_1 - NAV_0 + income + gains) / NAV_0.
double FundReturn(double nav0, double nav1, double income_distribution,
                  double capital_gain_distribution);
// Shares and value after reinvesting a distribution at NAV_1.
double SharesAfterReinvestment(double shares, double distribution_per_share,
                               double reinvest_nav);

// Terminal value of `amount` invested for `years` in a fund whose
// portfolio earns `gross_return` a year, with annual `expense_ratio`
// deducted from assets, a front load on purchase and a back-end
// (deferred) load on the redemption value. Back loads are given by year
// of redemption: back_loads[y-1] applies when redeeming at the end of
// year y; years beyond the schedule pay nothing.
double TerminalValue(double amount, double gross_return, double expense_ratio,
                     int years, double front_load,
                     std::span<const double> back_loads = {});

// Effective annual cost of fees: gross return minus the annualized net
// return implied by TerminalValue.
double AnnualizedFeeDrag(double gross_return, double expense_ratio, int years,
                         double front_load,
                         std::span<const double> back_loads = {});

// Smallest whole number of years for which fund A (e.g. a load fund with a
// low expense ratio) ends with at least as much as fund B; nullopt if not
// within `max_years`.
struct FundTerms {
  double expense_ratio;
  double front_load;
};
std::optional<int> BreakevenYears(const FundTerms& a, const FundTerms& b,
                                  double gross_return, int max_years = 100);

}  // namespace investm::ch21

#endif  // CPP_CH21_INVESTMENT_COMPANIES_FUNDS_H_
