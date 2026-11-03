"""Chapter 1, section 1.1.2: risk, return, and diversification.

Mirrors cpp/ch01_introduction/returns.h.
"""

import math
from typing import Optional, Sequence


def holding_period_return(begin_value: float,
                          end_value: float,
                          income: float = 0.0) -> float:
    """Returns (end value + income - begin value) / begin value."""
    if begin_value <= 0.0:
        raise ValueError("begin value must be positive")
    return (end_value + income - begin_value) / begin_value


def simple_returns(prices: Sequence[float],
                   income: Optional[Sequence[float]] = None) -> list[float]:
    """Period-by-period returns from a price series.

    Args:
        prices: At least two prices.
        income: Optional cash paid at the end of each period
            (len(prices) - 1 entries).

    Returns:
        A list of len(prices) - 1 simple returns.
    """
    if len(prices) < 2:
        raise ValueError("need at least two prices")
    if income is not None and len(income) != len(prices) - 1:
        raise ValueError("income must have len(prices) - 1 entries")
    return [
        holding_period_return(prices[t - 1], prices[t],
                              0.0 if income is None else income[t - 1])
        for t in range(1, len(prices))
    ]


def cumulative_return(returns: Sequence[float]) -> float:
    """Links periodic returns into one cumulative return."""
    growth = 1.0
    for r in returns:
        growth *= 1.0 + r
    return growth - 1.0


def annualize(cumulative: float, years: float) -> float:
    """Compound annual rate equivalent to `cumulative` over `years`."""
    if years <= 0.0:
        raise ValueError("years must be positive")
    return (1.0 + cumulative)**(1.0 / years) - 1.0


def equal_weight_portfolio_std_dev(n_assets: int, sigma: float,
                                   rho: float) -> float:
    """Volatility of an equal-weight portfolio of N alike assets.

    var_p = sigma^2 / N + (1 - 1/N) * rho * sigma^2. The first term is unique
    risk and vanishes as N grows; the second converges to rho * sigma^2 and
    cannot be diversified away.
    """
    if n_assets < 1:
        raise ValueError("need at least one asset")
    n = float(n_assets)
    return math.sqrt(sigma * sigma / n + (1.0 - 1.0 / n) * rho * sigma * sigma)
