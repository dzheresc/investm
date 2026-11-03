"""Chapter 6, sections 6.1 and 6.5: portfolio returns and moments.

Mirrors cpp/ch06_portfolio_selection/portfolio.h.
"""

import math
from typing import Sequence

import numpy as np


def weights_from_holdings(shares: Sequence[float],
                          prices: Sequence[float]) -> np.ndarray:
    values = np.asarray(shares, float) * np.asarray(prices, float)
    total = values.sum()
    if total == 0:
        raise ValueError("portfolio has zero value")
    return values / total


def portfolio_return_from_prices(shares: Sequence[float],
                                 begin_prices: Sequence[float],
                                 end_values: Sequence[float]) -> float:
    """(W1 - W0) / W0."""
    s = np.asarray(shares, float)
    w0 = float(s @ np.asarray(begin_prices, float))
    w1 = float(s @ np.asarray(end_values, float))
    if w0 <= 0:
        raise ValueError("initial wealth must be positive")
    return (w1 - w0) / w0


def portfolio_expected_return(weights, expected_returns) -> float:
    return float(np.asarray(weights, float) @ np.asarray(expected_returns))


def portfolio_variance(weights, cov) -> float:
    w = np.asarray(weights, float)
    return float(w @ np.asarray(cov, float) @ w)


def portfolio_std_dev(weights, cov) -> float:
    return math.sqrt(portfolio_variance(weights, cov))


def two_asset_std_dev(x1: float, s1: float, s2: float, rho: float) -> float:
    x2 = 1 - x1
    return math.sqrt(x1 * x1 * s1 * s1 + x2 * x2 * s2 * s2 +
                     2 * x1 * x2 * rho * s1 * s2)


def moments_from_scenarios(returns, probs) -> tuple[np.ndarray, np.ndarray]:
    """Expected returns and covariance matrix from joint scenarios.

    Args:
        returns: S x N matrix, one row per state.
        probs: Length-S state probabilities.

    Returns:
        (expected returns, covariance matrix).
    """
    r = np.asarray(returns, float)
    p = np.asarray(probs, float)
    mu = p @ r
    d = r - mu
    return mu, (d * p[:, None]).T @ d


def scenario_portfolio_returns(returns, weights) -> np.ndarray:
    return np.asarray(returns, float) @ np.asarray(weights, float)
