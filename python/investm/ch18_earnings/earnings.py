"""Chapter 18: earnings, dividends and earnings surprises.

Mirrors cpp/ch18_earnings/earnings.h.
"""

import dataclasses
from typing import Optional, Sequence

import numpy as np

from investm.core import regression
from investm.core import stats

_SEASON = 4


@dataclasses.dataclass(frozen=True)
class LintnerModel:
    a: float
    adjustment: float
    target_payout: float
    r_squared: float

    def next_dividend(self, previous_dividend: float, earnings: float) -> float:
        return previous_dividend + self.a + self.adjustment * (
            self.target_payout * earnings - previous_dividend)


def fit_lintner(earnings: Sequence[float],
                dividends: Sequence[float]) -> Optional[LintnerModel]:
    """Regresses dD_t on E_t and D_{t-1}."""
    if len(earnings) != len(dividends) or len(earnings) < 5:
        raise ValueError("need at least five matching years")
    e = np.asarray(earnings, dtype=float)
    d = np.asarray(dividends, dtype=float)
    x = np.column_stack([e[1:], d[:-1]])
    fit = regression.multiple_ols(x, d[1:] - d[:-1])
    if fit is None:
        return None
    c = -fit.coefficients[2]
    if c <= 0:
        return None
    return LintnerModel(float(fit.coefficients[0]), float(c),
                        float(fit.coefficients[1] / c), fit.r_squared)


def random_walk_forecast(annual: Sequence[float]) -> float:
    if len(annual) < 2:
        raise ValueError("need two years of earnings")
    return annual[-1] + (annual[-1] - annual[0]) / (len(annual) - 1)


def _seasonal_differences(q: Sequence[float]) -> list[float]:
    if len(q) < _SEASON + 2:
        raise ValueError("need at least six quarters of history")
    return [q[t] - q[t - _SEASON] for t in range(_SEASON, len(q))]


def seasonal_random_walk_forecast(quarterly: Sequence[float]) -> float:
    return quarterly[-_SEASON] + stats.mean(_seasonal_differences(quarterly))


def standardized_unexpected_earnings(history: Sequence[float],
                                     actual: float) -> float:
    sd = stats.std_dev(_seasonal_differences(history))
    if sd <= 0:
        raise ValueError("seasonal differences have no variation")
    return (actual - seasonal_random_walk_forecast(history)) / sd


def sort_into_groups(scores: Sequence[float], groups: int) -> list[int]:
    n = len(scores)
    if groups <= 0 or n < groups:
        raise ValueError("invalid group count")
    order = sorted(range(n), key=lambda i: scores[i])
    group = [0] * n
    for rank, i in enumerate(order):
        group[i] = rank * groups // n
    return group


def group_means(values: Sequence[float], group: Sequence[int],
                groups: int) -> list[float]:
    sums = [0.0] * groups
    counts = [0] * groups
    for v, g in zip(values, group):
        sums[g] += v
        counts[g] += 1
    return [s / c if c else 0.0 for s, c in zip(sums, counts)]
