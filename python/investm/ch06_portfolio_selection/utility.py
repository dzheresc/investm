"""Chapter 6, sections 6.2-6.4 and Appendix B: utility and risk aversion.

Mirrors cpp/ch06_portfolio_selection/utility.h.
"""

import enum
import math
from typing import Sequence

from investm.core import stats


class UtilityKind(enum.Enum):
    LOG = "log"  # ln W
    POWER = "power"  # W^(1-g) / (1-g)
    EXPONENTIAL = "exponential"  # -exp(-a W)
    QUADRATIC = "quadratic"  # W - b W^2
    LINEAR = "linear"  # W: risk neutral
    CONVEX = "convex"  # W^2: risk seeking


class Utility:
    """Utility of wealth with marginal utility, inverse and risk aversion."""

    def __init__(self, kind: UtilityKind, parameter: float = 0.0):
        if kind is UtilityKind.POWER and (parameter <= 0 or parameter == 1):
            raise ValueError("power utility needs g > 0, g != 1")
        if kind in (UtilityKind.EXPONENTIAL,
                    UtilityKind.QUADRATIC) and parameter <= 0:
            raise ValueError("parameter must be positive")
        self.kind = kind
        self.p = parameter

    def __call__(self, w: float) -> float:
        k, p = self.kind, self.p
        if k is UtilityKind.LOG:
            return math.log(w)
        if k is UtilityKind.POWER:
            return w**(1 - p) / (1 - p)
        if k is UtilityKind.EXPONENTIAL:
            return -math.exp(-p * w)
        if k is UtilityKind.QUADRATIC:
            if w >= 0.5 / p:
                raise ValueError("beyond the bliss point")
            return w - p * w * w
        if k is UtilityKind.LINEAR:
            return w
        return w * w

    def marginal(self, w: float) -> float:
        k, p = self.kind, self.p
        if k is UtilityKind.LOG:
            return 1 / w
        if k is UtilityKind.POWER:
            return w**(-p)
        if k is UtilityKind.EXPONENTIAL:
            return p * math.exp(-p * w)
        if k is UtilityKind.QUADRATIC:
            return 1 - 2 * p * w
        if k is UtilityKind.LINEAR:
            return 1.0
        return 2 * w

    def inverse(self, u: float) -> float:
        k, p = self.kind, self.p
        if k is UtilityKind.LOG:
            return math.exp(u)
        if k is UtilityKind.POWER:
            return (u * (1 - p))**(1 / (1 - p))
        if k is UtilityKind.EXPONENTIAL:
            return -math.log(-u) / p
        if k is UtilityKind.QUADRATIC:
            return (1 - math.sqrt(1 - 4 * p * u)) / (2 * p)
        if k is UtilityKind.LINEAR:
            return u
        return math.sqrt(u)

    def absolute_risk_aversion(self, w: float) -> float:
        k, p = self.kind, self.p
        if k is UtilityKind.LOG:
            return 1 / w
        if k is UtilityKind.POWER:
            return p / w
        if k is UtilityKind.EXPONENTIAL:
            return p
        if k is UtilityKind.QUADRATIC:
            return 2 * p / (1 - 2 * p * w)
        if k is UtilityKind.LINEAR:
            return 0.0
        return -1 / w


def expected_utility(u: Utility, wealth: Sequence[float],
                     probs: Sequence[float]) -> float:
    if len(wealth) != len(probs):
        raise ValueError("size mismatch")
    return math.fsum(p * u(w) for w, p in zip(wealth, probs))


def certainty_equivalent(u: Utility, wealth: Sequence[float],
                         probs: Sequence[float]) -> float:
    """Sure wealth valued as much as the gamble (6.3.2)."""
    return u.inverse(expected_utility(u, wealth, probs))


def risk_premium(u: Utility, wealth: Sequence[float],
                 probs: Sequence[float]) -> float:
    """Expected wealth minus certainty equivalent."""
    return stats.expected(wealth, probs) - certainty_equivalent(
        u, wealth, probs)


def mean_variance_utility(expected_return: float, std_dev: float,
                          tau: float) -> float:
    """U = E - sigma^2 / tau (6.4)."""
    if tau <= 0:
        raise ValueError("risk tolerance must be positive")
    return expected_return - std_dev**2 / tau


def indifference_curve(level: float, sigmas: Sequence[float],
                       tau: float) -> list[float]:
    """Expected returns giving utility `level` at each sigma."""
    if tau <= 0:
        raise ValueError("risk tolerance must be positive")
    return [level + s * s / tau for s in sigmas]
