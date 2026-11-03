"""Chapter 5, sections 5.1, 5.7, 5.8 and the continuous-compounding appendix.

Mirrors cpp/ch05_riskless_securities/rates.h.
"""

import math


def real_rate(nominal: float, inflation: float) -> float:
    """Fisher relation: (1 + nominal) = (1 + real)(1 + inflation)."""
    return (1.0 + nominal) / (1.0 + inflation) - 1.0


def nominal_rate(real: float, inflation: float) -> float:
    return (1.0 + real) * (1.0 + inflation) - 1.0


def real_rate_approx(nominal: float, inflation: float) -> float:
    return nominal - inflation


def _check_m(m: int) -> None:
    if m <= 0:
        raise ValueError("compounding frequency must be positive")


def effective_annual_rate(apr: float, m: int) -> float:
    _check_m(m)
    return (1.0 + apr / m)**m - 1.0


def apr_from_effective(effective: float, m: int) -> float:
    _check_m(m)
    return m * ((1.0 + effective)**(1.0 / m) - 1.0)


def convert_compounding(apr: float, m1: int, m2: int) -> float:
    return apr_from_effective(effective_annual_rate(apr, m1), m2)


def future_value(pv: float, apr: float, m: int, years: float) -> float:
    _check_m(m)
    return pv * (1.0 + apr / m)**(m * years)


def effective_from_continuous(rate: float) -> float:
    return math.expm1(rate)


def continuous_from_effective(effective: float) -> float:
    return math.log1p(effective)


def future_value_continuous(pv: float, rate: float, years: float) -> float:
    return pv * math.exp(rate * years)


def bill_price_from_discount(face: float, discount_yield: float,
                             days: int) -> float:
    """T-bill price from a bank discount quote: F (1 - d * days / 360)."""
    return face * (1.0 - discount_yield * days / 360.0)


def bank_discount_yield(face: float, price: float, days: int) -> float:
    return (face - price) / face * 360.0 / days


def bond_equivalent_yield(face: float, price: float, days: int) -> float:
    return (face - price) / price * 365.0 / days


def money_market_yield(face: float, price: float, days: int) -> float:
    return (face - price) / price * 360.0 / days


def bill_effective_annual_yield(face: float, price: float, days: int) -> float:
    return (face / price)**(365.0 / days) - 1.0
