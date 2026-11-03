"""Chapter 21: investment companies.

Mirrors cpp/ch21_investment_companies/funds.h.
"""

import dataclasses
from typing import Optional, Sequence


def net_asset_value(assets: float, liabilities: float, shares: float) -> float:
    if shares <= 0:
        raise ValueError("shares must be positive")
    return (assets - liabilities) / shares


def premium_or_discount(price: float, nav: float) -> float:
    return price / nav - 1.0


def offering_price(nav: float, front_load: float) -> float:
    if not 0 <= front_load < 1:
        raise ValueError("load must be in [0, 1)")
    return nav / (1.0 - front_load)


def load_as_fraction_of_investment(front_load: float) -> float:
    if not 0 <= front_load < 1:
        raise ValueError("load must be in [0, 1)")
    return front_load / (1.0 - front_load)


def fund_return(nav0: float, nav1: float, income: float, gains: float) -> float:
    return (nav1 - nav0 + income + gains) / nav0


def shares_after_reinvestment(shares: float, distribution: float,
                              reinvest_nav: float) -> float:
    return shares * (1.0 + distribution / reinvest_nav)


def terminal_value(
    amount: float,
    gross_return: float,
    expense_ratio: float,
    years: int,
    front_load: float,
    back_loads: Sequence[float] = ()) -> float:
    """Value after loads and an annual expense ratio deducted from assets."""
    if years < 0 or not 0 <= expense_ratio < 1 or not 0 <= front_load < 1:
        raise ValueError("invalid fund terms")
    value = amount * (1 - front_load)
    value *= ((1 + gross_return) * (1 - expense_ratio))**years
    if 1 <= years <= len(back_loads):
        value *= 1 - back_loads[years - 1]
    return value


def annualized_fee_drag(
    gross_return: float,
    expense_ratio: float,
    years: int,
    front_load: float,
    back_loads: Sequence[float] = ()) -> float:
    tv = terminal_value(1.0, gross_return, expense_ratio, years, front_load,
                        back_loads)
    return gross_return - (tv**(1.0 / years) - 1.0)


@dataclasses.dataclass(frozen=True)
class FundTerms:
    expense_ratio: float
    front_load: float


def breakeven_years(a: FundTerms,
                    b: FundTerms,
                    gross_return: float,
                    max_years: int = 100) -> Optional[int]:
    for y in range(1, max_years + 1):
        if (terminal_value(1, gross_return, a.expense_ratio, y, a.front_load)
                >= terminal_value(1, gross_return, b.expense_ratio, y,
                                  b.front_load)):
            return y
    return None
