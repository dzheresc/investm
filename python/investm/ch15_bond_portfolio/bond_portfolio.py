"""Chapter 15: bond portfolio management.

Mirrors cpp/ch15_bond_portfolio/bond_portfolio.h.
"""

import collections
import dataclasses
from typing import Optional, Sequence

from investm.ch05_riskless_securities import cash_flows as cf

_TIME_TOL = 1e-9


@dataclasses.dataclass(frozen=True)
class HorizonResult:
    purchase_price: float
    sale_price: float
    coupon_income: float
    interest_on_interest: float
    total_return: float
    realized_compound_yield: float


def horizon_analysis(face: float,
                     coupon_rate: float,
                     years_to_maturity: float,
                     buy_yield: float,
                     horizon: float,
                     end_yield: float,
                     reinvest_rate: float,
                     frequency: int = 2) -> HorizonResult:
    """Total return of buying at buy_yield and selling at end_yield."""
    if frequency <= 0 or not 0 < horizon <= years_to_maturity:
        raise ValueError("invalid horizon")
    h = round(horizon * frequency)
    if abs(h - horizon * frequency) > 1e-9:
        raise ValueError("horizon must be a whole number of periods")
    buy = cf.present_value_at_yield(
        cf.coupon_bond(face, coupon_rate, years_to_maturity, frequency),
        buy_yield, frequency)
    remaining = years_to_maturity - horizon
    sale = (cf.present_value_at_yield(
        cf.coupon_bond(face, coupon_rate, remaining, frequency), end_yield,
        frequency) if remaining > _TIME_TOL else face)
    coupon = face * coupon_rate / frequency
    g = 1.0 + reinvest_rate / frequency
    accumulated = sum(coupon * g**(h - k) for k in range(1, h + 1))
    end_value = sale + accumulated
    return HorizonResult(buy, sale, coupon * h, accumulated - coupon * h,
                         end_value / buy - 1.0,
                         frequency * ((end_value / buy)**(1.0 / h) - 1.0))


def portfolio_duration(values: Sequence[float],
                       durations: Sequence[float]) -> float:
    if len(values) != len(durations) or len(values) == 0:
        raise ValueError("size mismatch")
    total = sum(values)
    if total == 0:
        raise ValueError("portfolio value is zero")
    return sum(v * d for v, d in zip(values, durations)) / total


@dataclasses.dataclass(frozen=True)
class TwoBondHolding:
    value1: float
    value2: float


def immunize_two_bonds(liability_pv: float, liability_duration: float,
                       d1: float, d2: float) -> Optional[TwoBondHolding]:
    if d1 >= d2:
        raise ValueError("durations must satisfy d1 < d2")
    if not d1 <= liability_duration <= d2:
        return None
    w2 = (liability_duration - d1) / (d2 - d1)
    return TwoBondHolding(liability_pv * (1 - w2), liability_pv * w2)


def surplus(assets: Sequence[cf.CashFlow],
            liabilities: Sequence[cf.CashFlow],
            y: float,
            frequency: int = 1) -> float:
    return (cf.present_value_at_yield(assets, y, frequency) -
            cf.present_value_at_yield(liabilities, y, frequency))


def aggregate(bonds: Sequence[Sequence[cf.CashFlow]],
              units: Sequence[float]) -> list[cf.CashFlow]:
    if len(bonds) != len(units):
        raise ValueError("size mismatch")
    by_time: dict[float, float] = collections.defaultdict(float)
    for flows, q in zip(bonds, units):
        for f in flows:
            by_time[f.time] += q * f.amount
    return [cf.CashFlow(t, a) for t, a in sorted(by_time.items())]


def cash_match(bonds: Sequence[Sequence[cf.CashFlow]],
               liabilities: Sequence[cf.CashFlow]) -> Optional[list[float]]:
    """Backward-pass dedication; one bond must mature on each date."""
    owed = sorted(([f.time, f.amount] for f in liabilities), key=lambda x: x[0])
    units = [0.0] * len(bonds)
    for i in range(len(owed) - 1, -1, -1):
        t, need = owed[i]
        if need <= 0:
            continue
        pick = next((b for b, flows in enumerate(bonds)
                     if abs(flows[-1].time - t) < _TIME_TOL), None)
        if pick is None:
            return None
        q = need / bonds[pick][-1].amount
        units[pick] += q
        for f in bonds[pick]:
            for o in owed:
                if abs(o[0] - f.time) < _TIME_TOL:
                    o[1] -= q * f.amount
    return units


def required_immunized_assets(floor: float,
                              rate: float,
                              horizon: float,
                              frequency: int = 2) -> float:
    return floor / (1.0 + rate / frequency)**(frequency * horizon)


def contingent_cushion(assets: float,
                       floor: float,
                       rate: float,
                       horizon: float,
                       frequency: int = 2) -> float:
    return assets - required_immunized_assets(floor, rate, horizon, frequency)


def par_swap_rate(discount_factors: Sequence[float], period: float) -> float:
    if len(discount_factors) == 0 or period <= 0:
        raise ValueError("bad schedule")
    annuity = period * sum(discount_factors)
    return (1.0 - discount_factors[-1]) / annuity


def pay_fixed_swap_value(notional: float, fixed_rate: float,
                         discount_factors: Sequence[float],
                         period: float) -> float:
    if len(discount_factors) == 0 or period <= 0:
        raise ValueError("bad schedule")
    annuity = period * sum(discount_factors)
    return notional * (1.0 - discount_factors[-1] - fixed_rate * annuity)
