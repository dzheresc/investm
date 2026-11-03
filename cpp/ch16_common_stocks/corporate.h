// Chapter 16, Common Stocks.
// Section 16.1-16.4: the corporate form and corporate actions. Cumulative
// voting, stock splits and stock dividends, and rights offerings.

#ifndef CPP_CH16_COMMON_STOCKS_CORPORATE_H_
#define CPP_CH16_COMMON_STOCKS_CORPORATE_H_

#include <cstdint>
#include <span>
#include <vector>

namespace investm::ch16 {

// 16.1.3 Shares needed to be sure of electing `seats` of the `directors`
// being elected under cumulative voting, when `shares_voting` shares vote:
// floor(seats * shares / (directors + 1)) + 1.
std::int64_t CumulativeVotingSharesNeeded(std::int64_t shares_voting,
                                          int directors, int seats);
// Under majority (straight) voting a bloc needs more than half the shares
// to elect anyone.
std::int64_t MajorityVotingSharesNeeded(std::int64_t shares_voting);

// 16.3 Stock splits and stock dividends. A k-for-1 split (or a stock
// dividend of d, i.e. k = 1 + d) multiplies shares by k and, all else
// equal, divides the price by k.
double PriceAfterSplit(double price, double split_ratio);
inline double SplitRatioFromStockDividend(double dividend_fraction) {
  return 1.0 + dividend_fraction;
}
// Adjusts a price history for splits so returns are comparable: every
// price before date t with ratio k (applied at the start of day t) is
// divided by k. `ratios[t]` is 1 on days without a split.
std::vector<double> SplitAdjustPrices(std::span<const double> prices,
                                      std::span<const double> ratios);

// 16.4 Rights offerings: N old shares buy one new share at subscription
// price S.
struct RightsTerms {
  int rights_per_share;       // N
  double subscription_price;  // S
};
// Theoretical ex-rights price given the rights-on (cum-rights) price.
double ExRightsPrice(double cum_rights_price, const RightsTerms& terms);
// Value of one right: (P_cum - S) / (N + 1) = (P_ex - S) / N.
double RightValueCum(double cum_rights_price, const RightsTerms& terms);
double RightValueEx(double ex_rights_price, const RightsTerms& terms);

}  // namespace investm::ch16

#endif  // CPP_CH16_COMMON_STOCKS_CORPORATE_H_
