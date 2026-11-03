"""Chapter 5, sections 5.3-5.6 and 5.9-5.11: the term structure.

Rates are annually compounded and maturities are whole years. Mirrors
cpp/ch05_riskless_securities/term_structure.h.
"""

import dataclasses
from typing import Optional, Sequence

from investm.ch05_riskless_securities.cash_flows import CashFlow


class SpotCurve:
    """Spot rates s_1..s_n with discount factors and forward rates."""

    def __init__(self, spot_rates: Sequence[float]):
        if not spot_rates:
            raise ValueError("spot curve needs at least one rate")
        self._spot = list(spot_rates)

    @classmethod
    def from_discount_factors(cls, factors: Sequence[float]) -> "SpotCurve":
        if any(d <= 0.0 for d in factors):
            raise ValueError("discount factors must be positive")
        return cls([
            (1.0 / d)**(1.0 / t) - 1.0 for t, d in enumerate(factors, start=1)
        ])

    @classmethod
    def from_forwards(cls, one_year_forwards: Sequence[float]) -> "SpotCurve":
        """(1 + s_t)^t = (1 + f_{0,1}) ... (1 + f_{t-1,t})."""
        spot, growth = [], 1.0
        for t, f in enumerate(one_year_forwards, start=1):
            growth *= 1.0 + f
            spot.append(growth**(1.0 / t) - 1.0)
        return cls(spot)

    @property
    def max_maturity(self) -> int:
        return len(self._spot)

    @property
    def rates(self) -> list[float]:
        return list(self._spot)

    def spot(self, t: int) -> float:
        if not 1 <= t <= self.max_maturity:
            raise ValueError("maturity out of range")
        return self._spot[t - 1]

    def discount_factor(self, t: int) -> float:
        """d_t = (1 + s_t)^-t, with d_0 = 1."""
        return 1.0 if t == 0 else (1.0 + self.spot(t))**(-t)

    def forward(self, a: int, b: int) -> float:
        """(1 + f_{a,b})^(b - a) = d_a / d_b."""
        if not 0 <= a < b:
            raise ValueError("need 0 <= a < b")
        return (self.discount_factor(a) /
                self.discount_factor(b))**(1.0 / (b - a)) - 1.0

    def one_year_forward(self, t: int) -> float:
        return self.forward(t - 1, t)

    def present_value(self, flows: Sequence[CashFlow]) -> float:
        total = 0.0
        for cf in flows:
            t = round(cf.time)
            if abs(cf.time - t) > 1e-9:
                raise ValueError("cash flows must fall on whole years")
            total += cf.amount * self.discount_factor(t)
        return total

    def par_yield(self, t: int) -> float:
        """c = (1 - d_t) / (d_1 + ... + d_t)."""
        annuity = sum(self.discount_factor(k) for k in range(1, t + 1))
        return (1.0 - self.discount_factor(t)) / annuity


@dataclasses.dataclass(frozen=True)
class BondQuote:
    face: float
    coupon_rate: float  # Annual coupon, paid once a year.
    price: float


def bootstrap(quotes: Sequence[BondQuote]) -> Optional[SpotCurve]:
    """Spot rates from annual-coupon bonds maturing in years 1, 2, ..., n.

    Returns None if the prices imply a non-positive discount factor.
    """
    if not quotes:
        raise ValueError("no bond quotes")
    factors, annuity = [], 0.0
    for q in quotes:
        coupon = q.face * q.coupon_rate
        d = (q.price - coupon * annuity) / (coupon + q.face)
        if d <= 0.0:
            return None
        factors.append(d)
        annuity += d
    return SpotCurve.from_discount_factors(factors)


def expected_future_one_year_rates(curve: SpotCurve) -> list[float]:
    """Unbiased expectations: forwards are expected future spot rates."""
    return [curve.one_year_forward(t) for t in range(1, curve.max_maturity + 1)]


def liquidity_preference_curve(expected: Sequence[float],
                               premiums: Sequence[float]) -> SpotCurve:
    """f_{t-1,t} = E[future one-year rate] + liquidity premium L_t."""
    if len(expected) != len(premiums):
        raise ValueError("size mismatch")
    return SpotCurve.from_forwards([e + p for e, p in zip(expected, premiums)])


def implied_liquidity_premiums(curve: SpotCurve,
                               expected: Sequence[float]) -> list[float]:
    return [
        curve.one_year_forward(t) - expected[t - 1]
        for t in range(1, curve.max_maturity + 1)
    ]


def holding_period_return(buy_price: float,
                          sell_price: float,
                          coupons: float = 0.0) -> float:
    if buy_price <= 0.0:
        raise ValueError("buy price must be positive")
    return (sell_price + coupons - buy_price) / buy_price


def one_year_hpr_of_zero(today: SpotCurve, next_year: SpotCurve,
                         n: int) -> float:
    """Buy an n-year zero today, sell it after one year off next_year."""
    return holding_period_return(today.discount_factor(n),
                                 next_year.discount_factor(n - 1))


def roll_forward_one_year(curve: SpotCurve) -> SpotCurve:
    """Next year's curve if today's forward rates come true."""
    if curve.max_maturity < 2:
        raise ValueError("need at least two maturities")
    return SpotCurve(
        [curve.forward(1, t + 1) for t in range(1, curve.max_maturity)])
