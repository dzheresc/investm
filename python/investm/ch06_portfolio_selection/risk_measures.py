"""Chapter 6 institutional issues: alternative (downside) risk measures.

Mirrors cpp/ch06_portfolio_selection/risk_measures.h.
"""

import math
from statistics import NormalDist
from typing import Sequence

import numpy as np

_STD_NORMAL = NormalDist()


def semi_variance(returns: Sequence[float], target: float) -> float:
    r = np.asarray(returns, float)
    short = np.minimum(r - target, 0.0)
    return float((short**2).mean())


def downside_deviation(returns: Sequence[float], target: float) -> float:
    return math.sqrt(semi_variance(returns, target))


def shortfall_probability(returns: Sequence[float], target: float) -> float:
    return float((np.asarray(returns, float) < target).mean())


def _tail_count(n: int, alpha: float) -> int:
    if not 0 < alpha < 1:
        raise ValueError("alpha must be in (0, 1)")
    return max(1, math.ceil((1 - alpha) * n - 1e-9))


def historical_var(returns: Sequence[float], alpha: float) -> float:
    """Loss exceeded in only (1 - alpha) of observations (positive number)."""
    r = np.sort(np.asarray(returns, float))
    return float(-r[_tail_count(len(r), alpha) - 1])


def historical_expected_shortfall(returns: Sequence[float],
                                  alpha: float) -> float:
    r = np.sort(np.asarray(returns, float))
    return float(-r[:_tail_count(len(r), alpha)].mean())


def norm_inv(p: float) -> float:
    return _STD_NORMAL.inv_cdf(p)


def normal_var(mean: float, std_dev: float, alpha: float) -> float:
    return -(mean + std_dev * norm_inv(1 - alpha))


def normal_expected_shortfall(mean: float, std_dev: float,
                              alpha: float) -> float:
    z = norm_inv(1 - alpha)
    return -mean + std_dev * _STD_NORMAL.pdf(z) / (1 - alpha)
