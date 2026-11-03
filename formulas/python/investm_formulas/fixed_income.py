"""Taxes, growth, real returns and bonds (equations 25 to 34)."""

import math
from typing import Sequence

__all__ = [
    "break_even_tax_rate",
    "geometric_mean_growth_rate",
    "real_return",
    "approx_real_return",
    "bond_price",
    "yield_to_maturity",
    "discount_factor",
    "forward_rate",
    "macaulay_duration",
    "modified_duration",
    "effective_duration",
    "price_change_from_duration",
    "price_change_from_modified_duration",
]


def break_even_tax_rate(tax_exempt_yield: float, taxable_yield: float) -> float:
    """Eq. 25: t = 1 - tax-exempt yield / taxable yield."""
    return 1.0 - tax_exempt_yield / taxable_yield


def geometric_mean_growth_rate(beginning_value: float, ending_value: float,
                               years: float) -> float:
    """Eq. 26: g = (C_e / C_b)^(1/y) - 1."""
    return (ending_value / beginning_value)**(1.0 / years) - 1.0


def real_return(nominal_return: float, inflation: float) -> float:
    """Eq. 27 (exact): RR = (1 + NR) / (1 + CCL) - 1."""
    return (1.0 + nominal_return) / (1.0 + inflation) - 1.0


def approx_real_return(nominal_return: float, inflation: float) -> float:
    """Eq. 27 (approximation): RR ~ NR - CCL."""
    return nominal_return - inflation


def bond_price(coupons: Sequence[float], face_value: float,
               ytm: float) -> float:
    """Eq. 28: P_b = sum C_t / (1+y)^t + M / (1+y)^N.

    ``coupons`` holds C_1 .. C_N; the face value M is paid at N.
    """
    n = len(coupons)
    if n == 0:
        raise ValueError("need at least one period")
    pv = [c / (1.0 + ytm)**t for t, c in enumerate(coupons, start=1)]
    pv.append(face_value / (1.0 + ytm)**n)
    return math.fsum(pv)


def yield_to_maturity(price: float, coupons: Sequence[float],
                      face_value: float, low: float = -0.99, high: float = 10.0,
                      tolerance: float = 1e-12) -> float:
    """Eq. 28 solved for y by bisection (price falls as y rises)."""
    f_low = bond_price(coupons, face_value, low) - price
    f_high = bond_price(coupons, face_value, high) - price
    if f_low * f_high > 0.0:
        raise ValueError("price is not bracketed by [low, high]")
    for _ in range(200):
        mid = 0.5 * (low + high)
        f_mid = bond_price(coupons, face_value, mid) - price
        if f_mid == 0.0 or high - low < tolerance:
            return mid
        if (f_mid > 0.0) == (f_low > 0.0):
            low, f_low = mid, f_mid
        else:
            high = mid
    return 0.5 * (low + high)


def discount_factor(spot_rate: float, years: float) -> float:
    """Eq. 29: d_t = 1 / (1 + s_t)^t."""
    return 1.0 / (1.0 + spot_rate)**years


def forward_rate(spot_prev: float, spot_t: float, t: int) -> float:
    """Eq. 30: f_{t-1,t} = (1+s_t)^t / (1+s_{t-1})^(t-1) - 1."""
    return (1.0 + spot_t)**t / (1.0 + spot_prev)**(t - 1) - 1.0


def macaulay_duration(cash_flows: Sequence[float], ytm: float) -> float:
    """Eq. 31: D = sum PV(C_t) * t / P_0.

    ``cash_flows`` holds C_1 .. C_T, principal included in the last one.
    """
    pv = [c / (1.0 + ytm)**t for t, c in enumerate(cash_flows, start=1)]
    price = math.fsum(pv)
    return math.fsum(p * t for t, p in enumerate(pv, start=1)) / price


def modified_duration(duration: float, ytm: float) -> float:
    """Eq. 32: D_m = D / (1 + y)."""
    return duration / (1.0 + ytm)


def effective_duration(price_yield_down: float, price_yield_up: float,
                       price: float, yield_change: float) -> float:
    """Eq. 33: D_e = (P+ - P-) / (2 P dy).

    P+ is the price after the yield falls by dy, P- after it rises by dy.
    """
    return (price_yield_down - price_yield_up) / (2.0 * price * yield_change)


def price_change_from_duration(duration: float, ytm: float,
                               yield_change: float) -> float:
    """Eq. 34: dP/P = -D dy / (1 + y)."""
    return -duration * yield_change / (1.0 + ytm)


def price_change_from_modified_duration(modified: float,
                                        yield_change: float) -> float:
    """Eq. 34: dP/P = -D_m dy."""
    return -modified * yield_change
