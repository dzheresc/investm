#include "cpp/ch16_common_stocks/corporate.h"

#include <vector>

#include "cpp/core/check.h"

namespace investm::ch16 {

std::int64_t CumulativeVotingSharesNeeded(std::int64_t shares_voting,
                                          int directors, int seats) {
  INVESTM_CHECK(shares_voting > 0, "shares must be positive");
  INVESTM_CHECK(directors > 0 && seats >= 0 && seats <= directors,
                "seats must be in [0, directors]");
  return static_cast<std::int64_t>(seats) * shares_voting / (directors + 1) + 1;
}

std::int64_t MajorityVotingSharesNeeded(std::int64_t shares_voting) {
  INVESTM_CHECK(shares_voting > 0, "shares must be positive");
  return shares_voting / 2 + 1;
}

double PriceAfterSplit(double price, double split_ratio) {
  INVESTM_CHECK(split_ratio > 0.0, "split ratio must be positive");
  return price / split_ratio;
}

std::vector<double> SplitAdjustPrices(std::span<const double> prices,
                                      std::span<const double> ratios) {
  INVESTM_CHECK(prices.size() == ratios.size(), "size mismatch");
  std::vector<double> adjusted(prices.begin(), prices.end());
  double factor = 1.0;
  for (std::size_t t = prices.size(); t-- > 0;) {
    adjusted[t] /= factor;
    INVESTM_CHECK(ratios[t] > 0.0, "split ratio must be positive");
    factor *= ratios[t];
  }
  return adjusted;
}

double ExRightsPrice(double cum_rights_price, const RightsTerms& terms) {
  INVESTM_CHECK(terms.rights_per_share > 0, "N must be positive");
  const double n = terms.rights_per_share;
  return (n * cum_rights_price + terms.subscription_price) / (n + 1.0);
}

double RightValueCum(double cum_rights_price, const RightsTerms& terms) {
  INVESTM_CHECK(terms.rights_per_share > 0, "N must be positive");
  return (cum_rights_price - terms.subscription_price) /
         (terms.rights_per_share + 1.0);
}

double RightValueEx(double ex_rights_price, const RightsTerms& terms) {
  INVESTM_CHECK(terms.rights_per_share > 0, "N must be positive");
  return (ex_rights_price - terms.subscription_price) / terms.rights_per_share;
}

}  // namespace investm::ch16
