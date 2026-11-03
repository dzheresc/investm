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
    if not american:
        # Backward induction collapses to the terminal payoffs weighted by
        # C(n, j) p^j (1 - p)^(n - j): O(n) instead of O(n^2). The weights
        # are built in logs (cumulative ratios), so nothing underflows.
        j = np.arange(1, steps + 1)
        log_w = np.concatenate(
            ([steps * math.log1p(-p)], steps * math.log1p(-p) +
             np.cumsum(np.log((steps - j + 1) / j) + math.log(p / (1 - p)))))
        s = spec.spot * u**(2.0 * np.arange(steps + 1) - steps)
        terminal = np.maximum(sign * (s - spec.strike), 0.0)
        return float(
            math.exp(-spec.rate * spec.years) * (np.exp(log_w) @ terminal))
    # Node (i, j) has price S u^(2j - i): one step back multiplies by u, so
    # the early-exercise check reuses the terminal prices without powers.
    s = spec.spot * u**(2.0 * np.arange(steps + 1) - steps)
    v = np.maximum(sign * (s - spec.strike), 0.0)
    pu, pd = disc * p, disc * (1 - p)
    for i in range(steps - 1, -1, -1):
        v = pu * v[1:i + 2] + pd * v[:i + 1]
        if american:
            s = s[:i + 1] * u
            np.maximum(v, sign * (s - spec.strike), out=v)
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


class _BsmByVolatility:
    """Black-Scholes price and vega as functions of volatility alone.

    The discounted spot and strike are computed once, since the implied
    volatility search evaluates the same option many times.
    """

    def __init__(self, spec: OptionSpec):
        _check(spec)
        self.call = spec.option_type == OptionType.CALL
        self.spot_pv = spec.spot * math.exp(-spec.dividend_yield * spec.years)
        self.strike_pv = spec.strike * math.exp(-spec.rate * spec.years)
        self.log_moneyness = math.log(self.spot_pv / self.strike_pv)
        self.sqrt_t = math.sqrt(spec.years)

    def price(self, vol: float) -> float:
        sd = vol * self.sqrt_t
        d1 = self.log_moneyness / sd + 0.5 * sd
        d2 = d1 - sd
        if self.call:
            return (self.spot_pv * stats.norm_cdf(d1) -
                    self.strike_pv * stats.norm_cdf(d2))
        return (self.strike_pv * stats.norm_cdf(-d2) -
                self.spot_pv * stats.norm_cdf(-d1))

    def vega(self, vol: float) -> float:
        sd = vol * self.sqrt_t
        return self.spot_pv * stats.norm_pdf(self.log_moneyness / sd +
                                             0.5 * sd) * self.sqrt_t


def black_scholes_price(spec: OptionSpec) -> float:
    return _BsmByVolatility(spec).price(spec.volatility)


def implied_volatility(spec: OptionSpec, price: float) -> Optional[float]:
    """Newton from the inflection point, inside a bracket; Brent fallback."""
    bsm = _BsmByVolatility(spec)
    lower = bsm.price(1e-6)
    upper = bsm.spot_pv if bsm.call else bsm.strike_pv
    if not lower < price < upper:
        return None
    lo, hi = 1e-6, math.inf
    vol = max(math.sqrt(2.0 * abs(bsm.log_moneyness)) / bsm.sqrt_t, 0.1)
    for _ in range(50):
        diff = bsm.price(vol) - price
        if diff > 0:
            hi = vol
        else:
            lo = vol
        vega = bsm.vega(vol)
        nxt = vol - diff / vega if vega > 0 else -1.0
        if not lo < nxt < hi:
            break
        if abs(nxt - vol) <= 1e-15 * vol:
            return nxt
        vol = nxt
    f = lambda v: bsm.price(v) - price
    if math.isfinite(hi):
        return root_find.brent(f, lo, hi)
    return root_find.brent_expand_up(f, lo, max(2.0 * lo, 1.0))


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
