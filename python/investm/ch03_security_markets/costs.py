"""Chapter 3, sections 3.7 and 3.8: commissions and transactions costs.

Mirrors cpp/ch03_security_markets/costs.h.
"""

import dataclasses
import math


@dataclasses.dataclass(frozen=True)
class CommissionSchedule:
    per_ticket: float = 0.0
    per_share: float = 0.0
    fraction_of_principal: float = 0.0
    minimum: float = 0.0

    def commission(self, shares: float, price: float) -> float:
        c = (self.per_ticket + self.per_share * shares +
             self.fraction_of_principal * shares * price)
        return max(c, self.minimum)


@dataclasses.dataclass(frozen=True)
class Quote:
    bid: float
    ask: float

    @property
    def mid(self) -> float:
        return 0.5 * (self.bid + self.ask)

    @property
    def spread(self) -> float:
        return self.ask - self.bid

    @property
    def relative_spread(self) -> float:
        """Cost of an immediate round trip, as a fraction of the midpoint."""
        return self.spread / self.mid


def effective_half_spread(trade_price: float, quote: Quote) -> float:
    return abs(trade_price - quote.mid) / quote.mid


def sqrt_price_impact(shares: float,
                      average_daily_volume: float,
                      daily_vol: float,
                      k: float = 1.0) -> float:
    """Square-root impact rule of thumb: k * vol * sqrt(Q / ADV)."""
    if average_daily_volume <= 0.0:
        raise ValueError("ADV must be positive")
    return k * daily_vol * math.sqrt(shares / average_daily_volume)


def one_way_cost(shares: float, price: float, quote: Quote,
                 schedule: CommissionSchedule, impact_fraction: float) -> float:
    """Commission + half spread + impact, as a fraction of principal."""
    principal = shares * price
    return (schedule.commission(shares, price) / principal +
            0.5 * quote.relative_spread + impact_fraction)
