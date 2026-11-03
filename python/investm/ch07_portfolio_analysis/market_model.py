"""Chapter 7, sections 7.3-7.4 and Appendix B: the market model.

Mirrors cpp/ch07_portfolio_analysis/market_model.h.
"""

import dataclasses
from typing import Sequence

import numpy as np

from investm.core import stats


@dataclasses.dataclass(frozen=True)
class MarketModel:
    alpha: float
    beta: float
    residual_variance: float
    r_squared: float = 0.0

    def total_variance(self, index_variance: float) -> float:
        return self.beta**2 * index_variance + self.residual_variance


def fit_market_model(security: Sequence[float],
                     index: Sequence[float]) -> MarketModel:
    fit = stats.ols(index, security)
    return MarketModel(fit.alpha, fit.beta, fit.residual_std_dev**2,
                       fit.r_squared)


@dataclasses.dataclass(frozen=True)
class PortfolioRisk:
    beta: float
    alpha: float
    market_variance: float
    unique_variance: float
    total_variance: float


def portfolio_risk(weights: Sequence[float], models: Sequence[MarketModel],
                   index_variance: float) -> PortfolioRisk:
    """Market and unique risk, assuming uncorrelated residuals."""
    w = np.asarray(weights, float)
    beta = float(sum(x * m.beta for x, m in zip(w, models)))
    alpha = float(sum(x * m.alpha for x, m in zip(w, models)))
    unique = float(sum(x * x * m.residual_variance for x, m in zip(w, models)))
    market = beta * beta * index_variance
    return PortfolioRisk(beta, alpha, market, unique, market + unique)


def market_model_covariance(models: Sequence[MarketModel],
                            index_variance: float) -> np.ndarray:
    b = np.array([m.beta for m in models])
    return np.outer(b, b) * index_variance + np.diag(
        [m.residual_variance for m in models])


def market_model_expected_returns(models: Sequence[MarketModel],
                                  index_expected_return: float) -> np.ndarray:
    return np.array([m.alpha + m.beta * index_expected_return for m in models])


def random_diversification_variance(n: int, average_variance: float,
                                    average_covariance: float) -> float:
    if n < 1:
        raise ValueError("need at least one security")
    return average_variance / n + (1 - 1 / n) * average_covariance


def average_variance_and_covariance(cov) -> tuple[float, float]:
    c = np.asarray(cov, float)
    n = c.shape[0]
    off = (c.sum() - np.trace(c)) / 2
    return float(np.trace(c) / n), float(off / (n * (n - 1) / 2))
