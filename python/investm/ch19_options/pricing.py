"""Chapter 19, sections 19.4-19.7: binomial, Black-Scholes, warrants.

Mirrors cpp/ch19_options/pricing.h.
"""

import dataclasses
import math
from typing import Optional, Sequence

import numpy as np

from investm.ch19_options.payoffs import OptionType
from investm.ch19_options.payoffs import payoff
from investm.core import root_find
from investm.core import stats


@dataclasses.dataclass(frozen=True)
class Replication:
    shares: float
    bond: float
    value: float


def one_period_binomial(option_type: OptionType, spot: float, strike: float,
                        up: float, down: float, rate: float) -> Replication:
    if not down < 1 + rate < up:
        raise ValueError("need down < 1 + r < up")
    cu = payoff(option_type, strike, spot * up)
    cd = payoff(option_type, strike, spot * down)
    shares = (cu - cd) / (spot * (up - down))
    bond = (cu - shares * spot * up) / (1 + rate)
    return Replication(shares, bond, shares * spot + bond)


@dataclasses.dataclass(frozen=True)
class OptionSpec:
    option_type: OptionType
    spot: float
    strike: float
    rate: float
    volatility: float
    years: float
    dividend_yield: float = 0.0


def _check(s: OptionSpec) -> None:
    if s.spot <= 0 or s.strike <= 0 or s.volatility <= 0 or s.years <= 0:
        raise ValueError("invalid option spec")


def binomial_price(spec: OptionSpec, steps: int, american: bool) -> float:
    """Cox-Ross-Rubinstein tree, vectorized over nodes."""
    _check(spec)
    dt = spec.years / steps
    u = math.exp(spec.volatility * math.sqrt(dt))
    d = 1 / u
    p = (math.exp((spec.rate - spec.dividend_yield) * dt) - d) / (u - d)
    if not 0 < p < 1:
        raise ValueError("too few steps for these parameters")
    disc = math.exp(-spec.rate * dt)
    sign = 1.0 if spec.option_type == OptionType.CALL else -1.0
    j = np.arange(steps + 1)
    s = spec.spot * u**j * d**(steps - j)
    v = np.maximum(sign * (s - spec.strike), 0.0)
    for i in range(steps - 1, -1, -1):
        v = disc * (p * v[1:i + 2] + (1 - p) * v[:i + 1])
        if american:
            j = np.arange(i + 1)
            s = spec.spot * u**j * d**(i - j)
            v = np.maximum(v, sign * (s - spec.strike))
    return float(v[0])


@dataclasses.dataclass(frozen=True)
class Greeks:
    price: float
    delta: float
    gamma: float
    vega: float
    theta: float
    rho: float


def black_scholes_greeks(spec: OptionSpec) -> Greeks:
    _check(spec)
    s, k, t, sig = spec.spot, spec.strike, spec.years, spec.volatility
    q, r = spec.dividend_yield, spec.rate
    sqrt_t = math.sqrt(t)
    d1 = (math.log(s / k) + (r - q + 0.5 * sig * sig) * t) / (sig * sqrt_t)
    d2 = d1 - sig * sqrt_t
    dq, dr = math.exp(-q * t), math.exp(-r * t)
    pdf = stats.norm_pdf(d1)
    gamma = dq * pdf / (s * sig * sqrt_t)
    vega = s * dq * pdf * sqrt_t
    decay = -s * dq * pdf * sig / (2 * sqrt_t)
    if spec.option_type == OptionType.CALL:
        n1, n2 = stats.norm_cdf(d1), stats.norm_cdf(d2)
        return Greeks(s * dq * n1 - k * dr * n2, dq * n1, gamma, vega,
                      decay - r * k * dr * n2 + q * s * dq * n1,
                      k * t * dr * n2)
    n1, n2 = stats.norm_cdf(-d1), stats.norm_cdf(-d2)
    return Greeks(k * dr * n2 - s * dq * n1, -dq * n1, gamma, vega,
                  decay + r * k * dr * n2 - q * s * dq * n1, -k * t * dr * n2)


def black_scholes_price(spec: OptionSpec) -> float:
    return black_scholes_greeks(spec).price


def implied_volatility(spec: OptionSpec, price: float) -> Optional[float]:

    def at(vol: float) -> float:
        return black_scholes_price(dataclasses.replace(spec, volatility=vol))

    lower = at(1e-6)
    if spec.option_type == OptionType.CALL:
        upper = spec.spot * math.exp(-spec.dividend_yield * spec.years)
    else:
        upper = spec.strike * math.exp(-spec.rate * spec.years)
    if not lower < price < upper:
        return None
    return root_find.brent_expand_up(lambda v: at(v) - price, 1e-6, 1.0)


def historical_volatility(prices: Sequence[float],
                          periods_per_year: float) -> float:
    if len(prices) < 3:
        raise ValueError("need at least three prices")
    rel = [math.log(b / a) for a, b in zip(prices, prices[1:])]
    return stats.std_dev(rel) * math.sqrt(periods_per_year)


def warrant_value(spec: OptionSpec, shares: float, warrants: float) -> float:
    if spec.option_type != OptionType.CALL:
        raise ValueError("warrants are calls")
    return shares / (shares + warrants) * black_scholes_price(spec)
