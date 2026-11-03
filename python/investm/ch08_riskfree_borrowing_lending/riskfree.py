"""Chapter 8: riskfree lending and borrowing, tangency portfolio, O*.

Mirrors cpp/ch08_riskfree_borrowing_lending/riskfree.h.
"""

import dataclasses
import enum
import math
from typing import Optional

import numpy as np

from investm.ch07_portfolio_analysis import efficient_set as es


@dataclasses.dataclass(frozen=True)
class Combination:
    expected_return: float
    std_dev: float


def combine_with_riskfree(rf: float, risky_expected: float,
                          risky_std_dev: float, x_risky: float) -> Combination:
    """x_risky > 1 means borrowing at rf (8.3)."""
    return Combination((1 - x_risky) * rf + x_risky * risky_expected,
                       abs(x_risky) * risky_std_dev)


def sharpe_ratio(expected_return: float, std_dev: float, rf: float) -> float:
    if std_dev <= 0:
        raise ValueError("standard deviation must be positive")
    return (expected_return - rf) / std_dev


def tangency_portfolio(mu, cov, rf: float) -> Optional[es.PortfolioPoint]:
    """w proportional to S^-1 (mu - rf); None if no upper-branch tangency."""
    cov = np.asarray(cov, float)
    if np.linalg.matrix_rank(cov) < cov.shape[0]:
        return None
    z = np.linalg.solve(cov, np.asarray(mu, float) - rf)
    total = z.sum()
    if total <= 0:
        return None
    return es.evaluate(z / total, mu, cov)


def tangency_portfolio_no_short_sales(mu, cov, rf: float) -> es.PortfolioPoint:
    """Golden-section search for the max-Sharpe no-short-sales portfolio."""

    def sharpe(log_tau: float) -> float:
        p = es.optimal_no_short_sales(mu, cov, math.exp(log_tau))
        return (p.expected_return - rf) / p.std_dev if p.std_dev > 0 else -1e300

    inv_phi = (math.sqrt(5) - 1) / 2
    lo, hi = -12.0, 6.0
    x1, x2 = hi - inv_phi * (hi - lo), lo + inv_phi * (hi - lo)
    f1, f2 = sharpe(x1), sharpe(x2)
    for _ in range(80):
        if hi - lo <= 1e-7:
            break
        if f1 < f2:
            lo, x1, f1 = x1, x2, f2
            x2 = lo + inv_phi * (hi - lo)
            f2 = sharpe(x2)
        else:
            hi, x2, f2 = x2, x1, f1
            x1 = hi - inv_phi * (hi - lo)
            f1 = sharpe(x1)
    return es.optimal_no_short_sales(mu, cov, math.exp(0.5 * (lo + hi)))


def optimal_fraction_in_tangency(tangency: es.PortfolioPoint, rf: float,
                                 tau: float) -> float:
    """x = tau (E_T - rf) / (2 sigma_T^2) (Appendix B)."""
    return tau * (tangency.expected_return - rf) / (2 * tangency.std_dev**2)


class Region(enum.Enum):
    LEND = "lend"
    RISKY_ONLY = "risky only"
    BORROW = "borrow"


@dataclasses.dataclass(frozen=True)
class OptimalChoice:
    region: Region
    fraction_risky: float
    expected_return: float
    std_dev: float
    risky_weights: np.ndarray


def optimal_with_different_rates(mu, cov, r_lend: float, r_borrow: float,
                                 tau: float) -> Optional[OptimalChoice]:
    """Appendix A: lending rate below borrowing rate (short sales allowed)."""
    if r_lend > r_borrow:
        raise ValueError("lending rate must not exceed borrowing rate")
    t_lend = tangency_portfolio(mu, cov, r_lend)
    if t_lend is None:
        return None
    x = optimal_fraction_in_tangency(t_lend, r_lend, tau)
    if x <= 1:
        c = combine_with_riskfree(r_lend, t_lend.expected_return,
                                  t_lend.std_dev, x)
        return OptimalChoice(Region.LEND, x, c.expected_return, c.std_dev,
                             t_lend.weights)
    t_borrow = tangency_portfolio(mu, cov, r_borrow)
    x = (optimal_fraction_in_tangency(t_borrow, r_borrow, tau)
         if t_borrow is not None else 0.0)
    if x >= 1:
        c = combine_with_riskfree(r_borrow, t_borrow.expected_return,
                                  t_borrow.std_dev, x)
        return OptimalChoice(Region.BORROW, x, c.expected_return, c.std_dev,
                             t_borrow.weights)
    p = es.UnconstrainedFrontier.create(mu, cov).optimal(tau)
    return OptimalChoice(Region.RISKY_ONLY, 1.0, p.expected_return, p.std_dev,
                         p.weights)
