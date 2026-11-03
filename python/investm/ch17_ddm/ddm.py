"""Chapter 17: dividend discount models.

Mirrors cpp/ch17_ddm/ddm.h.
"""

from typing import Optional, Sequence

from investm.core import root_find


def present_value_of_dividends(dividends: Sequence[float], k: float) -> float:
    return sum(d / (1 + k)**(t + 1) for t, d in enumerate(dividends))


def zero_growth_value(dividend: float, k: float) -> float:
    if k <= 0:
        raise ValueError("required return must be positive")
    return dividend / k


def constant_growth_value(d0: float, g: float, k: float) -> float:
    if k <= g:
        raise ValueError("required return must exceed growth")
    return d0 * (1 + g) / (k - g)


def constant_growth_implied_return(price: float, d0: float, g: float) -> float:
    return d0 * (1 + g) / price + g


def multiple_growth_value(explicit_dividends: Sequence[float], g: float,
                          k: float) -> float:
    if len(explicit_dividends) == 0 or k <= g:
        raise ValueError("need dividends and k > g")
    t = len(explicit_dividends)
    terminal = explicit_dividends[-1] * (1 + g) / (k - g) / (1 + k)**t
    return present_value_of_dividends(explicit_dividends, k) + terminal


def multiple_growth_implied_return(price: float,
                                   explicit_dividends: Sequence[float],
                                   g: float) -> Optional[float]:
    return root_find.brent_expand_up(
        lambda k: multiple_growth_value(explicit_dividends, g, k) - price,
        g + 1e-9, g + 0.5)


def three_phase_dividends(d0: float, g_a: float, t1: int, t2: int,
                          g_c: float) -> list[float]:
    if not 0 <= t1 < t2:
        raise ValueError("need 0 <= t1 < t2")
    out = []
    d = d0
    for t in range(1, t2 + 1):
        g = g_a if t <= t1 else g_a - (g_a - g_c) * (t - t1) / (t2 - t1)
        d *= 1 + g
        out.append(d)
    return out


def three_phase_value(d0: float, g_a: float, t1: int, t2: int, g_c: float,
                      k: float) -> float:
    return multiple_growth_value(three_phase_dividends(d0, g_a, t1, t2, g_c),
                                 g_c, k)


def forward_price_earnings(payout: float, g: float, k: float) -> float:
    if k <= g:
        raise ValueError("required return must exceed growth")
    return payout / (k - g)


def trailing_price_earnings(payout: float, g: float, k: float) -> float:
    return forward_price_earnings(payout, g, k) * (1 + g)


def sustainable_growth(roe: float, retention: float) -> float:
    return roe * retention


def value_with_reinvestment(e1: float, retention: float, roe: float,
                            k: float) -> float:
    g = sustainable_growth(roe, retention)
    if k <= g:
        raise ValueError("required return must exceed growth")
    return e1 * (1 - retention) / (k - g)


def present_value_of_growth_opportunities(price: float, e1: float,
                                          k: float) -> float:
    return price - e1 / k
