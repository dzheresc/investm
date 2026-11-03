"""Chapter 4, sections 4.2.3 and 4.3: random walks and efficiency tests.

Mirrors cpp/ch04_efficient_markets/efficiency_tests.h.
"""

import dataclasses
import math
import random
from typing import Sequence

from investm.core import stats


def random_walk_prices(p0: float, n: int, mu: float, sigma: float,
                       seed: int) -> list[float]:
    """Geometric random walk with iid normal log returns; n + 1 prices."""
    rng = random.Random(seed)
    prices = [p0]
    for _ in range(n):
        prices.append(prices[-1] * math.exp(rng.gauss(mu, sigma)))
    return prices


def log_returns(prices: Sequence[float]) -> list[float]:
    return [math.log(b / a) for a, b in zip(prices, prices[1:])]


@dataclasses.dataclass(frozen=True)
class RunsTestResult:
    runs: int
    expected_runs: float
    z: float


def runs_test(returns: Sequence[float]) -> RunsTestResult:
    """Wald-Wolfowitz runs test on the signs of returns (zeros dropped)."""
    signs = [1 if r > 0 else -1 for r in returns if r != 0.0]
    if len(signs) < 2:
        raise ValueError("too few non-zero returns")
    n1 = float(sum(1 for s in signs if s > 0))
    n2 = float(len(signs)) - n1
    runs = 1 + sum(1 for a, b in zip(signs, signs[1:]) if a != b)
    n = n1 + n2
    mean = 2.0 * n1 * n2 / n + 1.0
    var = 2.0 * n1 * n2 * (2.0 * n1 * n2 - n) / (n * n * (n - 1.0))
    z = (runs - mean) / math.sqrt(var) if var > 0 else 0.0
    return RunsTestResult(runs, mean, z)


def variance_ratio(log_rets: Sequence[float], q: int) -> float:
    """VR(q) from overlapping q-period sums; about 1 under a random walk."""
    if q < 2 or len(log_rets) <= q:
        raise ValueError("invalid horizon q")
    sums = []
    window = 0.0
    for t, r in enumerate(log_rets):
        window += r
        if t >= q:
            window -= log_rets[t - q]
        if t + 1 >= q:
            sums.append(window)
    return stats.variance(sums) / (q * stats.variance(log_rets))


def filter_rule_return(prices: Sequence[float], x: float) -> float:
    """Buy after an x rise from a trough, sell after an x fall from a peak."""
    if len(prices) < 2:
        raise ValueError("need at least two prices")
    invested = False
    wealth, extreme, entry = 1.0, prices[0], 0.0
    for p in prices[1:]:
        if not invested:
            extreme = min(extreme, p)
            if p >= extreme * (1.0 + x):
                invested, entry, extreme = True, p, p
        else:
            extreme = max(extreme, p)
            if p <= extreme * (1.0 - x):
                invested = False
                wealth *= p / entry
                extreme = p
    if invested:
        wealth *= prices[-1] / entry
    return wealth - 1.0


@dataclasses.dataclass
class EventFirm:
    stock: list[float]
    market: list[float]


@dataclasses.dataclass(frozen=True)
class EventStudyResult:
    aar: list[float]
    caar: list[float]
    caar_t_stat: float


def event_study(firms: Sequence[EventFirm],
                estimation_length: int) -> EventStudyResult:
    """Market-model event study: AAR, CAAR and the t statistic of the CAAR."""
    if not firms:
        raise ValueError("no firms")
    total = len(firms[0].stock)
    if estimation_length >= total:
        raise ValueError("no event window")
    event_length = total - estimation_length
    aar = [0.0] * event_length
    residual_var_sum = 0.0
    for f in firms:
        if len(f.stock) != total or len(f.market) != total:
            raise ValueError("inconsistent series lengths")
        mm = stats.ols(f.market[:estimation_length],
                       f.stock[:estimation_length])
        residual_var_sum += mm.residual_std_dev**2
        for t in range(event_length):
            d = estimation_length + t
            aar[t] += f.stock[d] - mm.predict(f.market[d])
    n = float(len(firms))
    aar = [a / n for a in aar]
    caar = []
    cumulative = 0.0
    for a in aar:
        cumulative += a
        caar.append(cumulative)
    sd_caar = math.sqrt(event_length * residual_var_sum / (n * n))
    return EventStudyResult(aar, caar, cumulative / sd_caar)
