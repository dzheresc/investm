"""Chapter 20: futures.

Mirrors cpp/ch20_futures/futures.h.
"""

import dataclasses
import math
from typing import Sequence

from investm.ch19_options.payoffs import OptionType
from investm.core import stats


@dataclasses.dataclass(frozen=True)
class MarginDay:
    settlement_price: float
    gain: float
    margin_call: float
    equity: float


def mark_to_market(entry_price: float, settlements: Sequence[float],
                   position: float, multiplier: float, initial_margin: float,
                   maintenance_margin: float) -> list[MarginDay]:
    """Daily settlement; a call restores equity to the initial margin."""
    if not initial_margin >= maintenance_margin >= 0:
        raise ValueError("need initial >= maintenance >= 0")
    days = []
    equity = initial_margin
    prev = entry_price
    for price in settlements:
        gain = position * multiplier * (price - prev)
        equity += gain
        call = 0.0
        if equity < maintenance_margin:
            call = initial_margin - equity
            equity = initial_margin
        days.append(MarginDay(price, gain, call, equity))
        prev = price
    return days


def cost_of_carry_price(spot: float, rate: float, income_yield: float,
                        storage_cost: float, years: float) -> float:
    return spot * math.exp((rate + storage_cost - income_yield) * years)


@dataclasses.dataclass(frozen=True)
class Dividend:
    time: float
    amount: float


def index_futures_price(spot: float, rate: float, dividends: Sequence[Dividend],
                        years: float) -> float:
    pv = sum(d.amount * math.exp(-rate * d.time)
             for d in dividends
             if d.time <= years)
    return (spot - pv) * math.exp(rate * years)


@dataclasses.dataclass(frozen=True)
class ArbitrageBand:
    lower: float
    upper: float


def no_arbitrage_band(spot: float, lending_rate: float, borrowing_rate: float,
                      income_yield: float, cost: float,
                      years: float) -> ArbitrageBand:
    if borrowing_rate < lending_rate or cost < 0:
        raise ValueError("need borrowing >= lending and cost >= 0")
    return ArbitrageBand(
        spot * (1 - cost) * math.exp((lending_rate - income_yield) * years),
        spot * (1 + cost) * math.exp((borrowing_rate - income_yield) * years))


def basis(spot: float, futures: float) -> float:
    return spot - futures


def minimum_variance_hedge(
        spot_changes: Sequence[float],
        futures_changes: Sequence[float]) -> stats.SimpleRegression:
    return stats.ols(futures_changes, spot_changes)


def hedge_effectiveness(spot_changes: Sequence[float],
                        futures_changes: Sequence[float], h: float) -> float:
    hedged = [s - h * f for s, f in zip(spot_changes, futures_changes)]
    return 1 - stats.variance(hedged) / stats.variance(spot_changes)


def contracts_to_hedge(hedge_ratio: float, exposure: float,
                       contract_size: float) -> float:
    return hedge_ratio * exposure / contract_size


def index_futures_contracts(portfolio_value: float, current_beta: float,
                            target_beta: float, futures_price: float,
                            multiplier: float) -> float:
    return (target_beta - current_beta) * portfolio_value / (futures_price *
                                                             multiplier)


def black_futures_option_price(option_type: OptionType, futures: float,
                               strike: float, rate: float, volatility: float,
                               years: float) -> float:
    if futures <= 0 or strike <= 0 or volatility <= 0 or years <= 0:
        raise ValueError("invalid inputs")
    sd = volatility * math.sqrt(years)
    d1 = (math.log(futures / strike) + 0.5 * sd * sd) / sd
    d2 = d1 - sd
    disc = math.exp(-rate * years)
    if option_type == OptionType.CALL:
        return disc * (futures * stats.norm_cdf(d1) -
                       strike * stats.norm_cdf(d2))
    return disc * (strike * stats.norm_cdf(-d2) - futures * stats.norm_cdf(-d1))
