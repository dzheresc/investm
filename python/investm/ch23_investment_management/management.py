"""Chapter 23: risk tolerance and dynamic asset-mix strategies.

Mirrors cpp/ch23_investment_management/management.h.
"""

import dataclasses
from typing import Sequence


def implied_risk_tolerance(stock_weight: float, stock_expected: float,
                           stock_std_dev: float, riskfree: float) -> float:
    if stock_expected <= riskfree:
        raise ValueError("need a positive risk premium")
    return 2 * stock_std_dev**2 * stock_weight / (stock_expected - riskfree)


def optimal_stock_weight(tau: float, stock_expected: float,
                         stock_std_dev: float, riskfree: float) -> float:
    return tau * (stock_expected - riskfree) / (2 * stock_std_dev**2)


@dataclasses.dataclass
class StrategyPath:
    wealth: list[float]
    risky_weight: list[float] = dataclasses.field(default_factory=list)
    turnover: float = 0.0


def _grow(wealth: float, w: float, r: float, rf: float) -> tuple[float, float]:
    risky = wealth * w * (1 + r)
    safe = wealth * (1 - w) * (1 + rf)
    total = risky + safe
    return total, (risky / total if total else 0.0)


def buy_and_hold(risky_returns: Sequence[float],
                 riskfree: float,
                 initial_weight: float,
                 initial_wealth: float = 1.0) -> StrategyPath:
    p = StrategyPath([initial_wealth])
    w = initial_weight
    for r in risky_returns:
        p.risky_weight.append(w)
        nxt, w = _grow(p.wealth[-1], w, r, riskfree)
        p.wealth.append(nxt)
    return p


def constant_mix(risky_returns: Sequence[float],
                 riskfree: float,
                 target_weight: float,
                 band: float = 0.0,
                 initial_wealth: float = 1.0) -> StrategyPath:
    p = StrategyPath([initial_wealth])
    w = target_weight
    for r in risky_returns:
        if p.risky_weight and abs(w - target_weight) > band:
            p.turnover += abs(w - target_weight)
            w = target_weight
        p.risky_weight.append(w)
        nxt, w = _grow(p.wealth[-1], w, r, riskfree)
        p.wealth.append(nxt)
    return p


def cppi(risky_returns: Sequence[float],
         riskfree: float,
         multiplier: float,
         floor_fraction: float,
         max_weight: float = 1.0,
         initial_wealth: float = 1.0) -> StrategyPath:
    """Exposure = multiplier x cushion, capped; the floor earns riskfree."""
    p = StrategyPath([initial_wealth])
    floor = floor_fraction * initial_wealth
    drifted = 0.0
    for r in risky_returns:
        wealth = p.wealth[-1]
        cushion = max(wealth - floor, 0.0)
        w = min(max(multiplier * cushion /
                    wealth, 0.0), max_weight) if wealth > 0 else 0.0
        if p.risky_weight:
            p.turnover += abs(w - drifted)
        p.risky_weight.append(w)
        nxt, drifted = _grow(wealth, w, r, riskfree)
        p.wealth.append(nxt)
        floor *= 1 + riskfree
    return p
