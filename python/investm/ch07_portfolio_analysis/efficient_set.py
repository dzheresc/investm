"""Chapter 7, sections 7.1-7.2 and Appendix A: the Markowitz efficient set.

Mirrors cpp/ch07_portfolio_analysis/efficient_set.h. Short sales allowed:
closed form via A = 1'S^-1 1, B = 1'S^-1 mu, C = mu'S^-1 mu. No short
sales: accelerated projected gradient on the simplex.
"""

import dataclasses
import math
from typing import Optional, Sequence

import numpy as np


@dataclasses.dataclass(frozen=True)
class PortfolioPoint:
    weights: np.ndarray
    expected_return: float
    std_dev: float


def evaluate(weights, mu, cov) -> PortfolioPoint:
    w = np.asarray(weights, float)
    var = float(w @ np.asarray(cov, float) @ w)
    return PortfolioPoint(w, float(w @ np.asarray(mu, float)),
                          math.sqrt(max(var, 0.0)))


class UnconstrainedFrontier:
    """Minimum-variance frontier when short sales are allowed."""

    def __init__(self, mu, cov, inv):
        self.mu = np.asarray(mu, float)
        self.cov = np.asarray(cov, float)
        ones = np.ones(len(self.mu))
        self._inv_ones = inv @ ones
        self._inv_mu = inv @ self.mu
        self.a = float(ones @ self._inv_ones)
        self.b = float(ones @ self._inv_mu)
        self.c = float(self.mu @ self._inv_mu)
        self.d = self.a * self.c - self.b**2

    @classmethod
    def create(cls, mu, cov) -> Optional["UnconstrainedFrontier"]:
        """Returns None for a singular S or identical expected returns."""
        cov = np.asarray(cov, float)
        if np.linalg.matrix_rank(cov) < cov.shape[0]:
            return None
        f = cls(mu, cov, np.linalg.inv(cov))
        if f.d <= 1e-14 * f.a * f.c:
            return None
        return f

    def minimum_variance(self) -> PortfolioPoint:
        return evaluate(self._inv_ones / self.a, self.mu, self.cov)

    def for_target_return(self, target: float) -> PortfolioPoint:
        lam = (self.c - self.b * target) / self.d
        gam = (self.a * target - self.b) / self.d
        return evaluate(lam * self._inv_ones + gam * self._inv_mu, self.mu,
                        self.cov)

    def variance_at(self, target: float) -> float:
        return (self.a * target**2 - 2 * self.b * target + self.c) / self.d

    def optimal(self, tau: float) -> PortfolioPoint:
        """Maximizes E - sigma^2 / tau subject to full investment."""
        if tau < 0:
            raise ValueError("risk tolerance must be non-negative")
        eta = (1 - 0.5 * tau * self.b) / self.a
        return evaluate(0.5 * tau * self._inv_mu + eta * self._inv_ones,
                        self.mu, self.cov)


def project_onto_simplex(v: Sequence[float]) -> np.ndarray:
    """Euclidean projection onto {w >= 0, sum w = 1}."""
    v = np.asarray(v, float)
    u = np.sort(v)[::-1]
    css = np.cumsum(u) - 1.0
    ks = np.arange(1, len(v) + 1)
    rho = np.nonzero(u - css / ks > 0)[0][-1]
    theta = css[rho] / (rho + 1)
    return np.maximum(v - theta, 0.0)


def optimal_no_short_sales(mu,
                           cov,
                           tau: float,
                           max_iter: int = 20000,
                           tol: float = 1e-12) -> PortfolioPoint:
    """Maximizes tau mu'w - w'S w over the simplex (FISTA)."""
    mu = np.asarray(mu, float)
    cov = np.asarray(cov, float)
    if tau < 0:
        raise ValueError("risk tolerance must be non-negative")
    step = 1.0 / (2.0 * max(np.linalg.eigvalsh(cov)[-1], 1e-300))
    n = len(mu)
    w = np.full(n, 1.0 / n)
    y = w.copy()
    t = 1.0
    for _ in range(max_iter):
        w_next = project_onto_simplex(y - step * (2 * cov @ y - tau * mu))
        t_next = 0.5 * (1 + math.sqrt(1 + 4 * t * t))
        y = w_next + (t - 1) / t_next * (w_next - w)
        change = float(np.max(np.abs(w_next - w)))
        w, t = w_next, t_next
        if change < tol:
            break
    return evaluate(w, mu, cov)


def efficient_set_no_short_sales(mu, cov,
                                 taus: Sequence[float]) -> list[PortfolioPoint]:
    return [optimal_no_short_sales(mu, cov, t) for t in taus]


def two_asset_min_variance_weight(s1: float, s2: float, rho: float) -> float:
    denom = s1 * s1 + s2 * s2 - 2 * rho * s1 * s2
    if denom <= 0:
        raise ValueError("identical riskless combination")
    return (s2 * s2 - rho * s1 * s2) / denom
