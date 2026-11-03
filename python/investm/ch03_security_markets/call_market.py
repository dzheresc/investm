"""Chapter 3, section 3.1.1: clearing a call market.

Mirrors cpp/ch03_security_markets/call_market.h.
"""

import dataclasses
from typing import Sequence


@dataclasses.dataclass(frozen=True)
class LimitOrder:
    is_buy: bool
    limit: float
    quantity: float


@dataclasses.dataclass(frozen=True)
class CallAuctionResult:
    price: float
    volume: float
    imbalance: float


def demand_at(book: Sequence[LimitOrder], price: float) -> float:
    """Shares demanded at `price`: buy orders with limit >= price."""
    return sum(o.quantity for o in book if o.is_buy and o.limit >= price)


def supply_at(book: Sequence[LimitOrder], price: float) -> float:
    """Shares offered at `price`: sell orders with limit <= price."""
    return sum(o.quantity for o in book if not o.is_buy and o.limit <= price)


def clear_call_market(book: Sequence[LimitOrder]) -> CallAuctionResult:
    """Picks the limit price that maximizes executable volume.

    Ties go to the smallest absolute imbalance, then to the midpoint of the
    tied prices.
    """
    if not book:
        raise ValueError("empty order book")
    best = None  # (volume, -|imbalance|)
    low = high = 0.0
    for p in sorted({o.limit for o in book}):
        d, s = demand_at(book, p), supply_at(book, p)
        key = (min(d, s), -abs(d - s))
        if best is None or key > best:
            best, low, high = key, p, p
        elif key == best:
            high = p
    price = 0.5 * (low + high)
    d, s = demand_at(book, price), supply_at(book, price)
    return CallAuctionResult(price, min(d, s), d - s)
