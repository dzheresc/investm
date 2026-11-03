"""Chapter 16, sections 16.1-16.4: voting, splits and rights offerings.

Mirrors cpp/ch16_common_stocks/corporate.h.
"""

import dataclasses
from typing import Sequence


def cumulative_voting_shares_needed(shares_voting: int, directors: int,
                                    seats: int) -> int:
    if shares_voting <= 0 or directors <= 0 or not 0 <= seats <= directors:
        raise ValueError("invalid voting terms")
    return seats * shares_voting // (directors + 1) + 1


def majority_voting_shares_needed(shares_voting: int) -> int:
    if shares_voting <= 0:
        raise ValueError("shares must be positive")
    return shares_voting // 2 + 1


def price_after_split(price: float, split_ratio: float) -> float:
    if split_ratio <= 0:
        raise ValueError("split ratio must be positive")
    return price / split_ratio


def split_ratio_from_stock_dividend(dividend_fraction: float) -> float:
    return 1.0 + dividend_fraction


def split_adjust_prices(prices: Sequence[float],
                        ratios: Sequence[float]) -> list[float]:
    """Divides each price by the product of later split ratios."""
    if len(prices) != len(ratios):
        raise ValueError("size mismatch")
    adjusted = list(prices)
    factor = 1.0
    for t in range(len(prices) - 1, -1, -1):
        adjusted[t] /= factor
        factor *= ratios[t]
    return adjusted


@dataclasses.dataclass(frozen=True)
class RightsTerms:
    rights_per_share: int
    subscription_price: float


def ex_rights_price(cum_rights_price: float, terms: RightsTerms) -> float:
    n = terms.rights_per_share
    return (n * cum_rights_price + terms.subscription_price) / (n + 1)


def right_value_cum(cum_rights_price: float, terms: RightsTerms) -> float:
    return (cum_rights_price -
            terms.subscription_price) / (terms.rights_per_share + 1)


def right_value_ex(ex_price: float, terms: RightsTerms) -> float:
    return (ex_price - terms.subscription_price) / terms.rights_per_share
