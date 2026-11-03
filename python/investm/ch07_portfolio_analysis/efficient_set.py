"""Chapter 7, sections 7.1-7.2 and Appendix A: the Markowitz efficient set.

Mirrors cpp/ch07_portfolio_analysis/efficient_set.h. Short sales allowed:
closed form via A = 1'S^-1 1, B = 1'S^-1 mu, C = mu'S^-1 mu. No short
sales: accelerated projected gradient (with adaptive restart) on the
simplex.
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


def project_onto_affine_simplex(v: Sequence[float],
                                a: Sequence[float]) -> np.ndarray:
    """Euclidean projection onto {z >= 0, a'z = 1}; a may have any signs.

    z_i = max(0, v_i - theta a_i), where theta solves the monotone piecewise
    linear equation g(theta) = sum a_i max(0, v_i - theta a_i) = 1. Its
    breakpoints v_i / a_i are swept in order with cumulative sums.
    """
    v = np.asarray(v, float)
    a = np.asarray(a, float)
    if not np.any(a > 0):
        raise ValueError("a'z = 1 with z >= 0 needs some a_i > 0")
    nz = np.nonzero(a)[0]
    b = v[nz] / a[nz]
    order = np.argsort(b, kind="stable")
    idx, b = nz[order], b[order]
    pos = a > 0
    # Crossing b_i removes i from the active set if a_i > 0, adds it if < 0.
    sign = np.where(a[idx] > 0, -1.0, 1.0)
    s1 = float(a[pos] @ v[pos]) + np.concatenate(
        ([0.0], np.cumsum(sign * a[idx] * v[idx])))
    s2 = float(a[pos] @ a[pos]) + np.concatenate(
        ([0.0], np.cumsum(sign * a[idx]**2)))
    g = s1[:-1] - b * s2[:-1]  # g just left of each breakpoint.
    hit = np.nonzero(g <= 1.0)[0]
    if hit.size:
        k = hit[0]
        theta = (s1[k] - 1.0) / s2[k] if s2[k] > 0 else b[k]
    else:
        theta = (s1[-1] - 1.0) / s2[-1]
    return np.maximum(v - theta * a, 0.0)


def solve_affine_simplex_qp(cov,
                            c,
                            a,
                            lipschitz: float,
                            z0=None,
                            max_iter: int = 20000,
                            tol: float = 1e-12) -> np.ndarray:
    """Minimizes z'S z - c'z over {z >= 0, a'z = 1}.

    FISTA with gradient-based adaptive restart (O'Donoghue and Candes),
    started from z0 (warm start). `lipschitz` bounds 2 lambda_max(S).
    """
    cov = np.asarray(cov, float)
    c = np.asarray(c, float)
    a = np.asarray(a, float)
    step = 1.0 / max(lipschitz, 1e-300)
    if np.all(a == 1.0):  # Plain simplex: the cheaper cumsum projection.
        project = project_onto_simplex
    else:
        project = lambda x: project_onto_affine_simplex(x, a)
    w = project(np.full(len(c), 1.0 / len(c)) if z0 is None else z0)
    y = w.copy()
    t = 1.0
    for _ in range(max_iter):
        w_next = project(y - step * (2 * cov @ y - c))
        t_next = 0.5 * (1 + math.sqrt(1 + 4 * t * t))
        beta = (t - 1) / t_next
        if float((y - w_next) @ (w_next - w)) > 0:  # Momentum uphill.
            t_next, beta = 1.0, 0.0
        y = w_next + beta * (w_next - w)
        change = float(np.max(np.abs(w_next - w)))
        w, t = w_next, t_next
        if change < tol:
            break
    return w


def qp_lipschitz(cov) -> float:
    """2 lambda_max(S): the gradient's Lipschitz constant."""
    return 2.0 * float(np.linalg.eigvalsh(np.asarray(cov, float))[-1])


def optimal_no_short_sales(mu,
                           cov,
                           tau: float,
                           max_iter: int = 20000,
                           tol: float = 1e-12) -> PortfolioPoint:
    """Maximizes tau mu'w - w'S w over the simplex (FISTA with restart)."""
    if tau < 0:
        raise ValueError("risk tolerance must be non-negative")
    mu = np.asarray(mu, float)
    cov = np.asarray(cov, float)
    w = solve_affine_simplex_qp(cov,
                                tau * mu,
                                np.ones(len(mu)),
                                qp_lipschitz(cov),
                                max_iter=max_iter,
                                tol=tol)
    return evaluate(w, mu, cov)


def efficient_set_no_short_sales(mu, cov,
                                 taus: Sequence[float]) -> list[PortfolioPoint]:
    """Sweeps tau; one Lipschitz constant, each solve warm-starts the next."""
    mu = np.asarray(mu, float)
    cov = np.asarray(cov, float)
    lip = qp_lipschitz(cov)
    ones = np.ones(len(mu))
    w = None
    points = []
    for tau in taus:
        if tau < 0:
            raise ValueError("risk tolerance must be non-negative")
        w = solve_affine_simplex_qp(cov, tau * mu, ones, lip, w)
        points.append(evaluate(w, mu, cov))
    return points


def two_asset_min_variance_weight(s1: float, s2: float, rho: float) -> float:
    denom = s1 * s1 + s2 * s2 - 2 * rho * s1 * s2
    if denom <= 0:
        raise ValueError("identical riskless combination")
    return (s2 * s2 - rho * s1 * s2) / denom
