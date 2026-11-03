"""Chapter 16, section 16.7: estimating and adjusting betas.

Mirrors cpp/ch16_common_stocks/betas.h.
"""

from typing import Sequence

from investm.core import stats


def estimate_beta(security: Sequence[float],
                  market: Sequence[float]) -> stats.SimpleRegression:
    return stats.ols(market, security)


def rolling_betas(security: Sequence[float], market: Sequence[float],
                  window: int) -> list[float]:
    if len(security) != len(market) or window < 3:
        raise ValueError("invalid inputs")
    return [
        stats.ols(market[s:s + window], security[s:s + window]).beta
        for s in range(0,
                       len(security) - window + 1, window)
    ]


def shrink_beta(raw: float,
                weight: float = 2.0 / 3.0,
                prior: float = 1.0) -> float:
    if not 0 <= weight <= 1:
        raise ValueError("weight must be in [0, 1]")
    return weight * raw + (1.0 - weight) * prior


def vasicek_beta(raw: float, raw_std_error: float, prior_mean: float,
                 prior_std_dev: float) -> float:
    v_raw = raw_std_error**2
    v_prior = prior_std_dev**2
    return (v_raw * prior_mean + v_prior * raw) / (v_raw + v_prior)


def beta_persistence(earlier: Sequence[float],
                     later: Sequence[float]) -> stats.SimpleRegression:
    return stats.ols(earlier, later)


def levered_beta(unlevered: float, debt_to_equity: float,
                 tax_rate: float) -> float:
    return unlevered * (1.0 + (1.0 - tax_rate) * debt_to_equity)


def unlevered_beta(levered: float, debt_to_equity: float,
                   tax_rate: float) -> float:
    return levered / (1.0 + (1.0 - tax_rate) * debt_to_equity)
