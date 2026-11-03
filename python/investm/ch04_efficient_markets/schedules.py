"""Chapter 4, section 4.1: demand-to-buy and supply-to-sell schedules.

Mirrors cpp/ch04_efficient_markets/schedules.h.
"""

import dataclasses
from typing import Optional, Sequence

from investm.core import root_find


@dataclasses.dataclass
class Investor:
    value: float  # The investor's estimate of investment value.
    shares_held: float
    max_buy: float


def demand_to_buy(investors: Sequence[Investor], price: float,
                  sensitivity: float) -> float:
    """Shares wanted by investors who value the stock above `price`."""
    return sum(
        min(i.max_buy,
            sensitivity * (i.value - price) / price * i.max_buy)
        for i in investors
        if i.value > price)


def supply_to_sell(investors: Sequence[Investor], price: float,
                   sensitivity: float) -> float:
    """Shares offered by holders who value the stock below `price`."""
    return sum(
        min(i.shares_held,
            sensitivity * (price - i.value) / price * i.shares_held)
        for i in investors
        if i.value < price)


def equilibrium_price(investors: Sequence[Investor],
                      sensitivity: float = 5.0) -> Optional[float]:
    """Price where excess demand is zero; None if all valuations agree."""
    if not investors:
        return None
    lo = min(i.value for i in investors)
    hi = max(i.value for i in investors)
    if lo >= hi:
        return None
    return root_find.brent(
        lambda p: demand_to_buy(investors, p, sensitivity) - supply_to_sell(
            investors, p, sensitivity), lo, hi)


def arc_elasticity(p1: float, q1: float, p2: float, q2: float) -> float:
    """Midpoint (arc) elasticity between two points of a schedule."""
    dq = (q2 - q1) / (0.5 * (q1 + q2))
    dp = (p2 - p1) / (0.5 * (p1 + p2))
    return dq / dp
