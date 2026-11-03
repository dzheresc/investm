"""Chapter 14, sections 14.4-14.6: default risk and credit scoring.

Mirrors cpp/ch14_bond_analysis/credit.h.
"""

import dataclasses
from typing import Optional

import numpy as np

from investm.ch05_riskless_securities import cash_flows as cf


def expected_cash_flows(face: float, coupon_rate: float, years: int,
                        default_prob: float,
                        recovery: float) -> list[cf.CashFlow]:
    """Annual-pay bond; default recovers recovery * face and ends payments."""
    if years <= 0 or not 0 <= default_prob <= 1 or not 0 <= recovery <= 1:
        raise ValueError("invalid credit terms")
    flows = []
    survive = 1.0
    for t in range(1, years + 1):
        promised = face * coupon_rate + (face if t == years else 0.0)
        amount = (survive * (1 - default_prob) * promised +
                  survive * default_prob * recovery * face)
        flows.append(cf.CashFlow(float(t), amount))
        survive *= 1 - default_prob
    return flows


@dataclasses.dataclass(frozen=True)
class YieldDecomposition:
    promised_yield: float
    expected_yield: float
    default_premium: float
    risk_premium: float


def decompose_yield(face: float, coupon_rate: float, years: int, price: float,
                    default_prob: float, recovery: float,
                    riskfree: float) -> Optional[YieldDecomposition]:
    promised = cf.yield_to_maturity(cf.coupon_bond(face, coupon_rate, years),
                                    price)
    expected = cf.yield_to_maturity(
        expected_cash_flows(face, coupon_rate, years, default_prob, recovery),
        price)
    if promised is None or expected is None:
        return None
    return YieldDecomposition(promised, expected, promised - expected,
                              expected - riskfree)


def implied_default_probability(risky_yield: float, riskfree_yield: float,
                                recovery: float) -> float:
    if not 0 <= recovery < 1:
        raise ValueError("recovery must be in [0, 1)")
    return (1.0 - (1.0 + riskfree_yield) /
            (1.0 + risky_yield)) / (1.0 - recovery)


@dataclasses.dataclass(frozen=True)
class LinearDiscriminant:
    """Fisher discriminant; scores above `cutoff` look like group A."""
    weights: np.ndarray
    cutoff: float

    @staticmethod
    def fit(group_a, group_b) -> Optional["LinearDiscriminant"]:
        a = np.asarray(group_a, dtype=float)
        b = np.asarray(group_b, dtype=float)
        if a.shape[1] != b.shape[1] or len(a) < 2 or len(b) < 2:
            raise ValueError("need two firms per group, same ratios")
        ma, mb = a.mean(axis=0), b.mean(axis=0)
        s = ((a - ma).T @ (a - ma) +
             (b - mb).T @ (b - mb)) / (len(a) + len(b) - 2)
        try:
            w = np.linalg.solve(s, ma - mb)
        except np.linalg.LinAlgError:
            return None
        return LinearDiscriminant(w, float(w @ (0.5 * (ma + mb))))

    def score(self, x) -> float:
        return float(self.weights @ np.asarray(x, dtype=float))

    def is_group_a(self, x) -> bool:
        return self.score(x) > self.cutoff
