"""Chapter 12, section 12.2: inflation.

Mirrors cpp/ch12_taxes_inflation/inflation.h.
"""

from typing import Sequence

from investm.ch05_riskless_securities.cash_flows import CashFlow
from investm.core import stats


def inflation_rates(index_levels: Sequence[float]) -> list[float]:
    if len(index_levels) < 2:
        raise ValueError("need at least two index levels")
    return [b / a - 1.0 for a, b in zip(index_levels, index_levels[1:])]


def cumulative_inflation(index_levels: Sequence[float]) -> float:
    if len(index_levels) < 2:
        raise ValueError("need at least two index levels")
    return index_levels[-1] / index_levels[0] - 1.0


def purchasing_power(index_levels: Sequence[float]) -> float:
    return 1.0 / (1.0 + cumulative_inflation(index_levels))


def real_returns(nominal: Sequence[float],
                 inflation: Sequence[float]) -> list[float]:
    if len(nominal) != len(inflation):
        raise ValueError("size mismatch")
    return [(1.0 + r) / (1.0 + i) - 1.0 for r, i in zip(nominal, inflation)]


def fisher_nominal_rate(real: float, expected_inflation: float) -> float:
    return (1.0 + real) * (1.0 + expected_inflation) - 1.0


def real_after_tax_return(nominal: float, tax_rate: float,
                          inflation: float) -> float:
    return (1.0 + nominal * (1.0 - tax_rate)) / (1.0 + inflation) - 1.0


def effective_tax_on_real_return(nominal: float, tax_rate: float,
                                 inflation: float) -> float:
    real = (1.0 + nominal) / (1.0 + inflation) - 1.0
    if real <= 0:
        raise ValueError("pretax real return must be positive")
    return 1.0 - real_after_tax_return(nominal, tax_rate, inflation) / real


def indexed_bond_cash_flows(face: float,
                            real_coupon: float,
                            base_index: float,
                            index_path: Sequence[float],
                            frequency: int = 1) -> list[CashFlow]:
    """TIPS-style flows with a deflation floor on the final principal."""
    if frequency <= 0 or base_index <= 0 or len(index_path) == 0:
        raise ValueError("invalid indexed bond terms")
    flows = []
    for k, level in enumerate(index_path):
        principal = face * level / base_index
        amount = principal * real_coupon / frequency
        if k + 1 == len(index_path):
            amount += max(principal, face)
        flows.append(CashFlow((k + 1) / frequency, amount))
    return flows


def breakeven_inflation(nominal_yield: float, real_yield: float) -> float:
    return (1.0 + nominal_yield) / (1.0 + real_yield) - 1.0


def inflation_beta(nominal_returns: Sequence[float],
                   inflation: Sequence[float]) -> stats.SimpleRegression:
    return stats.ols(inflation, nominal_returns)
