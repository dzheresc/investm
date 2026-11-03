"""Chapter 14, sections 14.2-14.3: duration and convexity.

Mirrors cpp/ch14_bond_analysis/duration.h.
"""

from typing import Callable, Sequence

from investm.ch05_riskless_securities.cash_flows import CashFlow


def _moments(flows: Sequence[CashFlow], y: float,
             frequency: int) -> tuple[float, float, float]:
    if frequency <= 0:
        raise ValueError("frequency must be positive")
    g = 1.0 + y / frequency
    if g <= 0:
        raise ValueError("yield below -100% per period")
    price = time_weighted = convex = 0.0
    for f in flows:
        pv = f.amount / g**(f.time * frequency)
        price += pv
        time_weighted += f.time * pv
        convex += f.time * (f.time + 1.0 / frequency) * pv
    if price <= 0:
        raise ValueError("bond price must be positive")
    return price, time_weighted, convex


def macaulay_duration(flows: Sequence[CashFlow],
                      y: float,
                      frequency: int = 1) -> float:
    price, tw, _ = _moments(flows, y, frequency)
    return tw / price


def modified_duration(flows: Sequence[CashFlow],
                      y: float,
                      frequency: int = 1) -> float:
    return macaulay_duration(flows, y, frequency) / (1.0 + y / frequency)


def dollar_duration(flows: Sequence[CashFlow],
                    y: float,
                    frequency: int = 1) -> float:
    _, tw, _ = _moments(flows, y, frequency)
    return tw / (1.0 + y / frequency)


def price_value_of_basis_point(flows: Sequence[CashFlow],
                               y: float,
                               frequency: int = 1) -> float:
    return dollar_duration(flows, y, frequency) * 1e-4


def convexity(flows: Sequence[CashFlow], y: float, frequency: int = 1) -> float:
    price, _, cx = _moments(flows, y, frequency)
    g = 1.0 + y / frequency
    return cx / (price * g * g)


def approx_price_change(price: float, mod_duration: float, conv: float,
                        dy: float) -> float:
    return price * (-mod_duration * dy + 0.5 * conv * dy * dy)


def perpetuity_duration(y: float) -> float:
    if y <= 0:
        raise ValueError("yield must be positive")
    return (1.0 + y) / y


Pricer = Callable[[float], float]


def effective_duration(price: Pricer, y: float, dy: float = 1e-4) -> float:
    p0 = price(y)
    return (price(y - dy) - price(y + dy)) / (2.0 * p0 * dy)


def effective_convexity(price: Pricer, y: float, dy: float = 1e-4) -> float:
    p0 = price(y)
    return (price(y - dy) + price(y + dy) - 2.0 * p0) / (p0 * dy * dy)
