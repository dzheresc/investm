#!/usr/bin/env python3
"""Times investm against open-source libraries on the same inputs.

Each case computes one quantity from the book in every library that covers
it, with accuracy settings (steps, tolerances) matched as far as the
libraries allow, and reports the time per call next to the error against a
reference value. See docs/comparison_plan.md for the choice of libraries and
docs/comparison.md for the results.

Install the comparison libraries (they are not dependencies of investm):

    pip install -r scripts/compare/requirements.txt

Then, from the repository root:

    OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 \\
        python3 scripts/compare/compare_python.py \\
        --out docs/perf/compare-python.json

Timings are the best of --repeat runs of timeit's autorange (at least 0.2 s
each), as in scripts/bench_python.py. References are computed to 50 digits
with mpmath or exactly with fractions.Fraction, so "error" is the library's
own error and not a difference between two libraries.
"""

# pylint: disable=too-many-lines
import argparse
from collections.abc import Callable
import dataclasses
import datetime
import fractions
from importlib import metadata
import json
import math
import os
import platform
import sys
import timeit
from typing import Any, Optional
import warnings

import cvxpy as cp
import mpmath as mp
import numpy as np
import numpy_financial as npf
import pandas as pd
from pypfopt import black_litterman
from pypfopt import EfficientFrontier
import QuantLib as ql
from scipy import optimize
from scipy import special  # pylint: disable=no-name-in-module
from scipy import stats as sps
import statsmodels.api as sm
from statsmodels.sandbox.stats import runs as sm_runs
from statsmodels.tsa import stattools
from vollib import black_scholes as vollib_bs
from vollib.black_scholes import implied_volatility as vollib_iv
from vollib.black_scholes.greeks import analytical as vollib_greeks
from arch.unitroot import VarianceRatio

sys.path.insert(
    0,
    os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "..",
                 "python"))

# pylint: disable=wrong-import-position
from investm.ch04_efficient_markets import efficiency_tests as ch04
from investm.ch05_riskless_securities import cash_flows as ch05_cf
from investm.ch05_riskless_securities import term_structure as ch05_ts
from investm.ch06_portfolio_selection import risk_measures as ch06
from investm.ch07_portfolio_analysis import efficient_set as ch07
from investm.ch07_portfolio_analysis import market_model as ch07_mm
from investm.ch08_riskfree_borrowing_lending import riskfree as ch08
from investm.ch09_capm import capm as ch09
from investm.ch13_fixed_income import fixed_income as ch13
from investm.ch14_bond_analysis import duration as ch14
from investm.ch19_options import pricing as ch19
from investm.ch19_options.payoffs import OptionType
from investm.ch24_performance import performance as ch24
from investm.core import stats as core_stats

# pylint: enable=wrong-import-position

mp.mp.dps = 50
Fraction = fractions.Fraction

INVESTM = "investm"
QUANTLIB = "QuantLib"
VOLLIB = "vollib"
NPF = "numpy-financial"
SCIPY = "SciPy"
STATSMODELS = "statsmodels"
PYPFOPT = "PyPortfolioOpt"
ARCH = "arch"
CVXPY = "cvxpy"
PANDAS = "pandas"
NUMPY = "NumPy"


@dataclasses.dataclass
class Case:
    """One quantity computed by several libraries.

    Attributes:
      name: Short identifier, used by --filter and in the JSON.
      chapter: Book chapter the quantity comes from.
      title: One line describing the inputs and settings.
      reference_note: Where the reference comes from.
      runners: Library label to a zero-argument callable.
      error: Maps a runner's result to its error against the reference.
      show: Formats a runner's result for the value column.
      errors: Precomputed errors that override `error` (e.g. over a grid).
      notes: Library label to a note on how its settings differ.
    """

    name: str
    chapter: int
    title: str
    reference_note: str
    runners: dict[str, Callable[[], Any]]
    error: Callable[[Any], float]
    show: Callable[[Any], str] = lambda v: f"{v:.10g}"
    errors: dict[str, float] = dataclasses.field(default_factory=dict)
    notes: dict[str, str] = dataclasses.field(default_factory=dict)


# ---------------------------------------------------------------------------
# QuantLib helpers. A 30/360 day count on the 15th of the month makes whole
# months exact fractions of a year, so QuantLib's year fractions match
# investm's.

_QL_TODAY = ql.Date(15, ql.January, 2026)
_QL_DC = ql.Thirty360(ql.Thirty360.USA)
ql.Settings.instance().evaluationDate = _QL_TODAY


def _ql_flat(rate: float) -> ql.YieldTermStructureHandle:
    return ql.YieldTermStructureHandle(
        ql.FlatForward(_QL_TODAY, rate, _QL_DC, ql.Continuous))


def _ql_process(spot: float, rate: float,
                vol: float) -> ql.BlackScholesMertonProcess:
    return ql.BlackScholesMertonProcess(
        ql.QuoteHandle(ql.SimpleQuote(spot)), _ql_flat(0.0), _ql_flat(rate),
        ql.BlackVolTermStructureHandle(
            ql.BlackConstantVol(_QL_TODAY, ql.NullCalendar(), vol, _QL_DC)))


def _ql_npv(option: ql.VanillaOption, engine: Any) -> Callable[[], float]:
    """Prices on every call: setting the engine resets the cached NPV."""

    def run() -> float:
        option.setPricingEngine(engine)
        return option.NPV()

    return run


def _ql_schedule(start: ql.Date, end: ql.Date, freq: int) -> ql.Schedule:
    return ql.Schedule(start, end, ql.Period(freq), ql.NullCalendar(),
                       ql.Unadjusted, ql.Unadjusted, ql.DateGeneration.Backward,
                       False)


# ---------------------------------------------------------------------------
# High-precision references.


def _mp_ncdf(x: Any) -> Any:
    return mp.ncdf(x)


def _mp_bsm(s, k, r, v, t) -> dict[str, Any]:
    """Call price and Greeks to 50 digits (vega, rho per 1.00; theta/year)."""
    s, k, r, v, t = (mp.mpf(a) for a in (s, k, r, v, t))
    sq = mp.sqrt(t)
    d1 = (mp.log(s / k) + (r + v * v / 2) * t) / (v * sq)
    d2 = d1 - v * sq
    df = mp.exp(-r * t)
    pdf = mp.npdf(d1)
    return {
        "price": s * _mp_ncdf(d1) - k * df * _mp_ncdf(d2),
        "delta": _mp_ncdf(d1),
        "gamma": pdf / (s * v * sq),
        "vega": s * pdf * sq,
        "theta": -s * pdf * v / (2 * sq) - r * k * df * _mp_ncdf(d2),
        "rho": k * t * df * _mp_ncdf(d2),
    }


def _rel(a: float, b: Any) -> float:
    return float(abs((mp.mpf(a) - b) / b))


def _exact_ols(x_cols: list[np.ndarray], y: np.ndarray) -> list[Fraction]:
    """Least squares with an intercept, solved exactly in rationals."""
    cols = [[Fraction(1)] * len(y)
           ] + [[Fraction(float(v)) for v in c] for c in x_cols]
    yy = [Fraction(float(v)) for v in y]
    k = len(cols)
    a = [[sum(p * q
              for p, q in zip(cols[i], cols[j]))
          for j in range(k)] + [sum(p * q
                                    for p, q in zip(cols[i], yy))]
         for i in range(k)]
    for i in range(k):  # Gauss-Jordan; the normal matrix is SPD.
        for j in range(k):
            if j != i:
                f = a[j][i] / a[i][i]
                a[j] = [p - f * q for p, q in zip(a[j], a[i])]
    return [a[i][k] / a[i][i] for i in range(k)]


# ---------------------------------------------------------------------------
# Chapter 19: options.


def bsm_case() -> Case:
    s, k, r, v, t = 42.0, 40.0, 0.1, 0.2, 0.5
    spec = ch19.OptionSpec(OptionType.CALL, s, k, r, v, t)
    df, fwd, std = math.exp(-r * t), s * math.exp(r * t), v * math.sqrt(t)
    ref = _mp_bsm(s, k, r, v, t)["price"]
    return Case(
        "bsm_price",
        19,
        "BSM call, S=42 K=40 r=10% sigma=20% T=0.5",
        "closed form in mpmath (50 digits)", {
            INVESTM: lambda: ch19.black_scholes_price(spec),
            QUANTLIB: lambda: ql.blackFormula(ql.Option.Call, k, fwd, std, df),
            VOLLIB: lambda: vollib_bs.black_scholes("c", s, k, t, r, v),
        },
        lambda x: float(mp.mpf(x) - ref),
        notes={QUANTLIB: "blackFormula (function, no instrument objects)"})


def greeks_case() -> Case:
    s, k, r, v, t = 42.0, 40.0, 0.1, 0.2, 0.5
    spec = ch19.OptionSpec(OptionType.CALL, s, k, r, v, t)
    df, fwd, std = math.exp(-r * t), s * math.exp(r * t), v * math.sqrt(t)
    ref = _mp_bsm(s, k, r, v, t)

    def investm_greeks():
        g = ch19.black_scholes_greeks(spec)
        return (g.price, g.delta, g.gamma, g.vega, g.theta, g.rho)

    def ql_greeks():
        c = ql.BlackCalculator(ql.PlainVanillaPayoff(ql.Option.Call, k), fwd,
                               std, df)
        return (c.value(), c.delta(s), c.gamma(s), c.vega(t), c.theta(s, t),
                c.rho(t))

    def vollib_all():
        # vollib quotes vega and rho per 1% and theta per day.
        return (vollib_bs.black_scholes("c", s, k, t, r, v),
                vollib_greeks.delta("c", s, k, t, r,
                                    v), vollib_greeks.gamma("c", s, k, t, r, v),
                100 * vollib_greeks.vega("c", s, k, t, r, v),
                365 * vollib_greeks.theta("c", s, k, t, r, v),
                100 * vollib_greeks.rho("c", s, k, t, r, v))

    keys = ("price", "delta", "gamma", "vega", "theta", "rho")
    return Case("bsm_greeks",
                19,
                "Price and five Greeks of the same call, one call for all six",
                "closed forms in mpmath; error is the largest relative error", {
                    INVESTM: investm_greeks,
                    QUANTLIB: ql_greeks,
                    VOLLIB: vollib_all,
                },
                lambda g: max(_rel(x, ref[key]) for x, key in zip(g, keys)),
                show=lambda g: f"delta {g[1]:.6f}, theta {g[4]:.4f}",
                notes={
                    QUANTLIB: "BlackCalculator",
                    VOLLIB: "six separate calls; rescaled to per 1.00 and "
                            "per year",
                })


def implied_vol_case() -> Case:
    price, s, k, r, t = 1.875, 21.0, 20.0, 0.1, 0.25
    spec = ch19.OptionSpec(OptionType.CALL, s, k, r, 0.2, t)
    df, fwd = math.exp(-r * t), s * math.exp(r * t)
    ref = mp.findroot(lambda vol: _mp_bsm(s, k, r, vol, t)["price"] - price,
                      mp.mpf("0.23"))

    def ql_iv(accuracy: float) -> Callable[[], float]:
        return lambda: ql.blackFormulaImpliedStdDev(
            ql.Option.Call, k, fwd, price, df, 0.0, ql.nullDouble(
            ), accuracy) / math.sqrt(t)

    return Case(
        "implied_vol",
        19,
        "Implied vol of a call at 1.875, S=21 K=20 r=10% T=0.25",
        "mpmath root of the 50-digit price", {
            INVESTM:
                lambda: ch19.implied_volatility(spec, price),
            QUANTLIB:
                ql_iv(1e-6),
            QUANTLIB + " (1e-12)":
                ql_iv(1e-12),
            VOLLIB:
                lambda: vollib_iv.implied_volatility(price, s, k, t, r, "c"),
        },
        lambda x: float(mp.mpf(x) - ref),
        notes={
            INVESTM: "Brent, tolerance 1e-12",
            QUANTLIB: "blackFormulaImpliedStdDev, default accuracy",
            VOLLIB: "Let's Be Rational",
        })


def _crr_ql(steps: int, american: bool) -> Callable[[], float]:
    expiry = _QL_TODAY + ql.Period(5, ql.Months)
    exercise = (ql.AmericanExercise(_QL_TODAY, expiry)
                if american else ql.EuropeanExercise(expiry))
    option = ql.VanillaOption(ql.PlainVanillaPayoff(ql.Option.Put, 50.0),
                              exercise)
    engine = ql.BinomialVanillaEngine(_ql_process(50.0, 0.1, 0.4), "crr", steps)
    return _ql_npv(option, engine)


def crr_cases() -> list[Case]:
    spec = ch19.OptionSpec(OptionType.PUT, 50.0, 50.0, 0.1, 0.4, 5.0 / 12.0)
    ref_am = 0.5 * (ch19.binomial_price(spec, 20000, True) +
                    ch19.binomial_price(spec, 20001, True))
    s, k, r, v, t = 50.0, 50.0, 0.1, 0.4, 5.0 / 12.0
    call = _mp_bsm(s, k, r, v, t)["price"]
    ref_eu = call - s + k * mp.exp(-r * t)  # Put-call parity.
    cases = []
    for steps in (100, 1000):
        cases.append(
            Case(
                f"crr_american_{steps}",
                19, f"American put on a {steps}-step CRR tree, S=K=50 r=10% "
                "sigma=40% T=5/12",
                f"mean of 20,000- and 20,001-step trees, {ref_am:.6f}", {
                    INVESTM: lambda n=steps: ch19.binomial_price(spec, n, True),
                    QUANTLIB: _crr_ql(steps, True),
                },
                lambda x: x - ref_am,
                show=lambda x: f"{x:.6f}"))
    cases.append(
        Case("crr_european_1000",
             19,
             "European put on a 1,000-step CRR tree, same option",
             "Black-Scholes put in mpmath", {
                 INVESTM: lambda: ch19.binomial_price(spec, 1000, False),
                 QUANTLIB: _crr_ql(1000, False),
             },
             lambda x: float(mp.mpf(x) - ref_eu),
             show=lambda x: f"{x:.6f}",
             notes={
                 INVESTM: "closed-form binomial sum, O(n)",
                 QUANTLIB: "backward induction, O(n^2)",
             }))
    return cases


# ---------------------------------------------------------------------------
# Chapters 6 and 19: the normal distribution.


def norm_cdf_case() -> Case:
    grid = np.linspace(-8.0, 8.0, 1001)
    ql_n = ql.CumulativeNormalDistribution()
    funcs = {
        INVESTM: core_stats.norm_cdf,
        QUANTLIB: ql_n,
        SCIPY + " (special.ndtr)": special.ndtr,  # pylint: disable=no-member
        SCIPY + " (stats.norm.cdf)": sps.norm.cdf,
    }
    refs = [_mp_ncdf(mp.mpf(x)) for x in grid]
    errors = {
        name: max(_rel(float(f(x)), ref) for x, ref in zip(grid, refs))
        for name, f in funcs.items()
    }
    return Case("norm_cdf",
                6, "N(x) at one point (x = -1.5); error over 1,001 points on "
                "[-8, 8]",
                "mpmath ncdf; error is the largest relative error", {
                    name: lambda f=f: f(-1.5) for name, f in funcs.items()
                },
                lambda x: 0.0,
                errors=errors)


def norm_inv_case() -> Case:
    grid = [10.0**(-10.0 + i / 50.0) for i in range(500)] + [0.5]
    ql_n = ql.InverseCumulativeNormal()
    funcs = {
        INVESTM: ch06.norm_inv,
        QUANTLIB: ql_n,
        SCIPY + " (special.ndtri)": special.ndtri,  # pylint: disable=no-member
        SCIPY + " (stats.norm.ppf)": sps.norm.ppf,
    }
    refs = [mp.sqrt(2) * mp.erfinv(2 * mp.mpf(p) - 1) for p in grid]
    errors = {
        name:
            max(
                float(abs(mp.mpf(float(f(p))) - ref))
                for p, ref in zip(grid, refs)) for name, f in funcs.items()
    }
    return Case("norm_inv",
                6, "Inverse N(p) at p = 0.05 (95% VaR); error over p in "
                "[1e-10, 0.5]",
                "mpmath; error is the largest absolute error in z", {
                    name: lambda f=f: f(0.05) for name, f in funcs.items()
                },
                lambda x: 0.0,
                errors=errors,
                notes={INVESTM: "statistics.NormalDist (Wichura AS241)"})


# ---------------------------------------------------------------------------
# Chapters 5, 13 and 14: bonds and cash flows.

_FACE, _COUPON, _YIELD = 100.0, 0.08, 0.09


def _ql_bond() -> ql.FixedRateBond:
    sched = _ql_schedule(_QL_TODAY, _QL_TODAY + ql.Period(10, ql.Years),
                         ql.Semiannual)
    return ql.FixedRateBond(0, _FACE, sched, [_COUPON], _QL_DC, ql.Unadjusted)


def _exact_bond(y: Fraction) -> tuple[Fraction, Fraction, Fraction]:
    """Price, Macaulay duration and convexity of the 10-year 8% bond."""
    g = 1 + y / 2
    price = tw = cx = Fraction(0)
    for k in range(1, 21):
        t = Fraction(k, 2)
        pv = (4 + (100 if k == 20 else 0)) / g**k
        price += pv
        tw += t * pv
        cx += t * (t + Fraction(1, 2)) * pv
    return price, tw / price, cx / (price * g * g)


def bond_price_case() -> Case:
    flows = ch05_cf.coupon_bond(_FACE, _COUPON, 10, 2)
    bond = _ql_bond()
    y = ql.InterestRate(_YIELD, _QL_DC, ql.Compounded, ql.Semiannual)
    ref = _exact_bond(Fraction(_YIELD))[0]
    stream = [0.0] + [4.0] * 19 + [104.0]
    return Case(
        "bond_price",
        5,
        "10-year 8% semiannual bond at a 9% yield, on a coupon date",
        "exact rational sum", {
            INVESTM: lambda: ch05_cf.present_value_at_yield(flows, _YIELD, 2),
            QUANTLIB: lambda: ql.BondFunctions.cleanPrice(bond, y, _QL_TODAY),
            NPF + " (pv)": lambda: -npf.pv(_YIELD / 2, 20, 4.0, 100.0),
            NPF + " (npv)": lambda: npf.npv(_YIELD / 2, stream),
        },
        lambda x: float(Fraction(x) - ref),
        notes={NPF + " (pv)": "annuity closed form"})


def bond_yield_case() -> Case:
    flows = ch05_cf.coupon_bond(_FACE, _COUPON, 10, 2)
    price = float(_exact_bond(Fraction(_YIELD))[0])
    bond = _ql_bond()
    ql_price = ql.BondPrice(price, ql.BondPrice.Clean)
    ref = mp.findroot(
        lambda y: sum((4 + (100 if k == 20 else 0)) / (1 + y / 2)**k
                      for k in range(1, 21)) - mp.mpf(price), mp.mpf("0.09"))

    def ql_yield(accuracy: float) -> Callable[[], float]:
        return lambda: ql.BondFunctions.bondYield(
            bond, ql_price, _QL_DC, ql.Compounded, ql.Semiannual, _QL_TODAY,
            accuracy)

    def scipy_yield() -> float:
        return optimize.brentq(lambda y: -npf.pv(y / 2, 20, 4.0, 100.0) - price,
                               0.0,
                               1.0,
                               xtol=1e-12)

    return Case(
        "bond_yield",
        5,
        "Yield to maturity of the same bond from its price",
        "mpmath root", {
            INVESTM:
                lambda: ch05_cf.yield_to_maturity(flows, price, 2),
            QUANTLIB:
                ql_yield(1e-10),
            QUANTLIB + " (1e-12)":
                ql_yield(1e-12),
            NPF:
                lambda: 2 * float(npf.rate(20, 4.0, -price, 100.0)),
            NPF + " (tol 1e-12)":
                lambda: 2 * float(npf.rate(20, 4.0, -price, 100.0, tol=1e-12)),
            SCIPY:
                scipy_yield,
        },
        lambda x: float(mp.mpf(x) - ref),
        notes={
            INVESTM: "Brent, tolerance 1e-12",
            QUANTLIB: "bondYield, default accuracy 1e-10",
            NPF: "rate (Newton), default tol 1e-6",
            SCIPY: "brentq (xtol 1e-12) on numpy-financial's pv",
        })


def duration_case() -> Case:
    flows = ch05_cf.coupon_bond(_FACE, _COUPON, 10, 2)
    bond = _ql_bond()
    y = ql.InterestRate(_YIELD, _QL_DC, ql.Compounded, ql.Semiannual)
    _, mac, conv = _exact_bond(Fraction(_YIELD))
    mod = mac / (1 + Fraction(_YIELD) / 2)
    refs = (mac, mod, conv)

    def investm_all():
        return (ch14.macaulay_duration(flows, _YIELD, 2),
                ch14.modified_duration(flows, _YIELD,
                                       2), ch14.convexity(flows, _YIELD, 2))

    def ql_all():
        f = ql.BondFunctions
        return (f.duration(bond, y, ql.Duration.Macaulay, _QL_TODAY),
                f.duration(bond, y, ql.Duration.Modified,
                           _QL_TODAY), f.convexity(bond, y, _QL_TODAY))

    return Case("duration",
                14,
                "Macaulay and modified duration and convexity of the same bond",
                "exact rationals; error is the largest relative error", {
                    INVESTM: investm_all,
                    QUANTLIB: ql_all,
                },
                lambda v: max(
                    float(abs((Fraction(x) - r) / r)) for x, r in zip(v, refs)),
                show=lambda v: f"{v[0]:.6f}, {v[1]:.6f}, {v[2]:.4f}")


def treasury_case() -> Case:
    bond = ch13.Bond(_FACE, 0.07, datetime.date(2036, 11, 15), 2,
                     ch13.DayCount.ACTUAL_ACTUAL)
    settle = datetime.date(2027, 3, 1)
    sched = _ql_schedule(ql.Date(15, ql.November, 2026),
                         ql.Date(15, ql.November, 2036), ql.Semiannual)
    dc = ql.ActualActual(ql.ActualActual.ISMA, sched)
    ql_bond = ql.FixedRateBond(0, _FACE, sched, [0.07], dc, ql.Unadjusted)
    ql_settle = ql.Date(1, ql.March, 2027)
    y = ql.InterestRate(0.065, dc, ql.Compounded, ql.Semiannual)
    # Street convention: 106 of 181 days have run since 15 November 2026.
    accrued = Fraction(35, 10) * Fraction(106, 181)
    w = mp.mpf(75) / 181
    g = 1 + mp.mpf("0.065") / 2
    dirty = sum((mp.mpf("3.5") + (100 if k == 19 else 0)) / g**(k + w)
                for k in range(20))
    ref = dirty - mp.mpf(accrued.numerator) / accrued.denominator

    def investm_both():
        return (ch13.accrued_interest(bond, settle),
                ch13.clean_price(bond, settle, 0.065))

    def ql_both():
        ql.Settings.instance().evaluationDate = ql_settle
        return (ql_bond.accruedAmount(ql_settle),
                ql.BondFunctions.cleanPrice(ql_bond, y, ql_settle))

    return Case(
        "treasury_clean_price",
        13, "7% Treasury maturing 2036-11-15, settled 2027-03-01, act/act, "
        "accrued interest and clean price at 6.5%",
        "mpmath street-convention price; error is the larger absolute error", {
            INVESTM: investm_both,
            QUANTLIB: ql_both,
        },
        lambda v: max(float(abs(Fraction(v[0]) - accrued)),
                      float(abs(mp.mpf(v[1]) - ref))),
        show=lambda v: f"{v[0]:.6f}, {v[1]:.6f}",
        notes={QUANTLIB: "ActualActual(ISMA) with the coupon schedule"})


def _true_discount(t: int) -> float:
    s = 0.03 + 0.02 * (1.0 - math.exp(-t / 5.0))
    return (1.0 + s)**-t


def bootstrap_case() -> Case:
    quotes, annuity = [], 0.0
    for n in range(1, 31):
        c = 0.04 + 0.0005 * n
        annuity += _true_discount(n)
        quotes.append(
            ch05_ts.BondQuote(100.0, c,
                              100.0 * (c * annuity + _true_discount(n))))
    ql.Settings.instance().evaluationDate = _QL_TODAY
    helpers = [
        ql.FixedRateBondHelper(
            ql.QuoteHandle(ql.SimpleQuote(q.price)), 0, 100.0,
            _ql_schedule(_QL_TODAY, _QL_TODAY + ql.Period(i + 1, ql.Years),
                         ql.Annual), [q.coupon_rate], _QL_DC, ql.Unadjusted)
        for i, q in enumerate(quotes)
    ]

    def investm_curve():
        curve = ch05_ts.bootstrap(quotes)
        return [curve.discount_factor(t) for t in range(1, 31)]

    def ql_curve():
        ql.Settings.instance().evaluationDate = _QL_TODAY
        curve = ql.PiecewiseLogLinearDiscount(_QL_TODAY, helpers, _QL_DC)
        return [curve.discount(float(t)) for t in range(1, 31)]

    return Case(
        "bootstrap",
        5, "Spot curve from 30 annual-coupon bonds priced off a known "
        "curve",
        "the known curve; error is the largest discount-factor error", {
            INVESTM: investm_curve,
            QUANTLIB: ql_curve,
        },
        lambda d: max(abs(x - _true_discount(t)) for t, x in enumerate(d, 1)),
        show=lambda d: f"d30 = {d[-1]:.8f}",
        notes={
            INVESTM: "one linear solve per maturity",
            QUANTLIB: "PiecewiseLogLinearDiscount on "
                      "FixedRateBondHelper",
        })


def irr_case() -> Case:
    values = [
        -1000.0, 100.0, 150.0, -50.0, 200.0, 250.0, 300.0, -100.0, 400.0, 350.0,
        600.0
    ]
    flows = [ch05_cf.CashFlow(float(t), v) for t, v in enumerate(values)]
    ref = mp.findroot(
        lambda r: sum(mp.mpf(v) / (1 + r)**t for t, v in enumerate(values)),
        mp.mpf("0.1"))
    return Case(
        "irr",
        24, "IRR of an 11-period stream with sign changes (a "
        "dollar-weighted return)",
        "mpmath root", {
            INVESTM:
                lambda: ch05_cf.internal_rate_of_return(flows),
            NPF:
                lambda: float(npf.irr(values)),
            SCIPY:
                lambda: optimize.brentq(
                    lambda r: npf.npv(r, values), -0.99, 1.0, xtol=1e-12),
        },
        lambda x: float(mp.mpf(x) - ref),
        notes={
            INVESTM: "Brent on [-0.99, 1], tolerance 1e-12",
            NPF: "eigenvalues of the companion matrix",
            SCIPY: "brentq on numpy-financial's npv",
        })


# ---------------------------------------------------------------------------
# Chapters 7, 8, 9 and 24: portfolio optimization.


def _portfolio_problem(n: int) -> tuple[np.ndarray, np.ndarray]:
    """Same construction as scripts/bench_python.py, seeded by n."""
    rng = np.random.default_rng(n)
    mu = rng.uniform(0.04, 0.16, n)
    b = rng.normal(size=(n + 20, n))
    return mu, b.T @ b / (n + 20) * 0.04


def _sharpe(w, mu, cov, rf):
    w = np.asarray(w, float)
    return float((w @ mu - rf) / math.sqrt(w @ cov @ w))


def _pypfopt_weights(ef: EfficientFrontier) -> np.ndarray:
    return np.array(list(ef.clean_weights(cutoff=0.0, rounding=None).values()))


def _slsqp(objective, grad, n, bounds) -> np.ndarray:
    res = optimize.minimize(objective,
                            np.full(n, 1.0 / n),
                            jac=grad,
                            method="SLSQP",
                            bounds=bounds,
                            constraints=[{
                                "type": "eq",
                                "fun": lambda w: w.sum() - 1.0,
                                "jac": np.ones_like,
                            }],
                            options={
                                "ftol": 1e-15,
                                "maxiter": 1000
                            })
    return res.x


def tangency_case() -> Case:
    mu, cov = _portfolio_problem(20)
    rf = 0.03
    z = np.linalg.solve(cov, mu - rf)
    best = _sharpe(z / z.sum(), mu, cov, rf)

    def pypfopt():
        ef = EfficientFrontier(mu, cov, weight_bounds=(None, None))
        ef.max_sharpe(risk_free_rate=rf)
        return _pypfopt_weights(ef)

    return Case(
        "tangency",
        8,
        "Tangency portfolio, 20 assets, short sales allowed",
        "closed form S^-1 (mu - rf) with numpy; error is the Sharpe ratio "
        "shortfall", {
            INVESTM: lambda: ch08.tangency_portfolio(mu, cov, rf).weights,
            PYPFOPT: pypfopt,
        },
        lambda w: best - _sharpe(w, mu, cov, rf),
        show=lambda w: f"Sharpe {_sharpe(w, mu, cov, rf):.10f}",
        notes={PYPFOPT: "max_sharpe through cvxpy (Clarabel)"})


def _long_only_cases() -> list[Case]:
    rf, tau = 0.03, 0.3
    cases = []
    for n in (20, 100):
        mu, cov = _portfolio_problem(n)
        bounds = ((0.0, 1.0),) * n

        def investm_tangency(mu=mu, cov=cov):
            return ch08.tangency_portfolio_no_short_sales(mu, cov, rf).weights

        def pypfopt_tangency(mu=mu, cov=cov):
            ef = EfficientFrontier(mu, cov)
            ef.max_sharpe(risk_free_rate=rf)
            return _pypfopt_weights(ef)

        def scipy_tangency(mu=mu, cov=cov, bounds=bounds):

            def f(w):
                return -_sharpe(w, mu, cov, rf)

            def g(w):
                s = math.sqrt(w @ cov @ w)
                e = w @ mu - rf
                return -(mu / s - e * (cov @ w) / s**3)

            return _slsqp(f, g, len(mu), bounds)

        runners = {
            INVESTM: investm_tangency,
            PYPFOPT: pypfopt_tangency,
            SCIPY: scipy_tangency,
        }
        best = max(_sharpe(r(), mu, cov, rf) for r in runners.values())
        cases.append(
            Case(f"tangency_long_only_{n}",
                 8,
                 f"Max-Sharpe portfolio, {n} assets, no short sales",
                 "best Sharpe ratio found by any library; error is the "
                 "shortfall",
                 runners,
                 lambda w, mu=mu, cov=cov, best=best: best - _sharpe(
                     w, mu, cov, rf),
                 show=lambda w, mu=mu, cov=cov:
                 f"Sharpe {_sharpe(w, mu, cov, rf):.10f}",
                 notes={
                     INVESTM: "one QP: min z'Sz, (mu - rf)'z = 1, z >= 0",
                     PYPFOPT: "max_sharpe (same transformation, cvxpy)",
                     SCIPY: "SLSQP on -Sharpe, analytic gradient",
                 }))

        def utility(w, mu=mu, cov=cov):
            w = np.asarray(w, float)
            return float(tau * w @ mu - w @ cov @ w)

        def pypfopt_utility(mu=mu, cov=cov):
            ef = EfficientFrontier(mu, cov)
            ef.max_quadratic_utility(risk_aversion=2.0 / tau)
            return _pypfopt_weights(ef)

        def scipy_utility(mu=mu, cov=cov, bounds=bounds):
            return _slsqp(lambda w: -(tau * w @ mu - w @ cov @ w),
                          lambda w: -(tau * mu - 2.0 * cov @ w), len(mu),
                          bounds)

        runners = {
            INVESTM:
                lambda mu=mu, cov=cov: ch07.optimal_no_short_sales(
                    mu, cov, tau).weights,
            PYPFOPT:
                pypfopt_utility,
            SCIPY:
                scipy_utility,
        }
        best = max(utility(r()) for r in runners.values())
        cases.append(
            Case(f"optimal_long_only_{n}",
                 7, f"Optimal portfolio for risk tolerance 0.3 "
                 f"(max tau mu'w - w'Sw), {n} assets, no short sales",
                 "best objective found by any library; error is the "
                 "shortfall",
                 runners,
                 lambda w, best=best, utility=utility: best - utility(w),
                 show=lambda w, utility=utility: f"objective {utility(w):.12f}",
                 notes={
                     INVESTM: "FISTA with adaptive restart",
                     PYPFOPT: "max_quadratic_utility, risk aversion 2 / tau",
                     SCIPY: "SLSQP, analytic gradient",
                 }))
    return cases


def implied_returns_case() -> Case:
    mu, cov = _portfolio_problem(20)
    del mu
    w = np.random.default_rng(3).dirichlet(np.ones(20))
    rf, premium = 0.03, 0.06
    delta = premium / float(w @ cov @ w)
    caps, cov_df = pd.Series(w), pd.DataFrame(cov)
    ref = [
        mp.mpf(rf) + mp.mpf(premium) *
        sum(mp.mpf(cov[i, j]) * mp.mpf(w[j])
            for j in range(20)) / sum(
                mp.mpf(w[a]) * mp.mpf(cov[a, b]) * mp.mpf(w[b])
                for a in range(20)
                for b in range(20))
        for i in range(20)
    ]
    return Case(
        "implied_returns",
        9,
        "CAPM-implied expected returns (reverse optimization), 20 assets",
        "mpmath; error is the largest absolute error", {
            INVESTM:
                lambda: ch09.implied_expected_returns(cov, w, rf, premium),
            PYPFOPT:
                lambda: black_litterman.market_implied_prior_returns(
                    caps, delta, cov_df, rf).to_numpy(),
        },
        lambda v: max(float(abs(mp.mpf(x) - r)) for x, r in zip(v, ref)),
        show=lambda v: f"first {v[0]:.8f}",
        notes={
            PYPFOPT: "market_implied_prior_returns, delta = premium / "
                     "var(market)"
        })


def style_case() -> Case:
    rng = np.random.default_rng(1)
    x = rng.normal(0.01, 0.04, size=(120, 8))
    fund = x @ np.full(8, 1 / 8) + rng.normal(0, 0.005, 120)
    xc = x - x.mean(axis=0)
    yc = fund - fund.mean()

    def tracking_var(w):
        return float(np.var(fund - x @ np.asarray(w, float), ddof=1))

    def cvxpy_fit():
        w = cp.Variable(8)
        cp.Problem(cp.Minimize(cp.sum_squares(yc - xc @ w)),
                   [w >= 0, cp.sum(w) == 1]).solve(solver=cp.CLARABEL)
        return w.value

    def scipy_fit():
        s = xc.T @ xc
        c = xc.T @ yc
        return _slsqp(lambda w: w @ s @ w - 2 * c @ w, lambda w: 2 *
                      (s @ w - c), 8, [(0.0, 1.0)] * 8)

    runners = {
        INVESTM: lambda: ch24.style_analysis(fund, x).weights,
        CVXPY: cvxpy_fit,
        SCIPY: scipy_fit,
    }
    best = min(tracking_var(r()) for r in runners.values())
    return Case("style_analysis",
                24,
                "Sharpe style analysis, 120 months x 8 styles (weights >= 0, "
                "sum 1)",
                "smallest tracking variance found; error is the excess",
                runners,
                lambda w: tracking_var(w) - best,
                show=lambda w: f"tracking var {tracking_var(w):.10e}",
                notes={
                    INVESTM: "FISTA with adaptive restart",
                    CVXPY: "sum_squares QP, Clarabel",
                    SCIPY: "SLSQP, analytic gradient",
                })


# ---------------------------------------------------------------------------
# Chapters 4, 7 and 24: regressions and tests of market efficiency.


def market_model_case() -> Case:
    rng = np.random.default_rng(11)
    index = rng.normal(0.01, 0.045, 60)
    security = 0.002 + 1.2 * index + rng.normal(0.0, 0.03, 60)
    alpha, beta = _exact_ols([index], security)

    def investm_fit():
        m = ch07_mm.fit_market_model(security, index)
        return (m.alpha, m.beta)

    def sm_fit():
        p = sm.OLS(security, sm.add_constant(index)).fit().params
        return (p[0], p[1])

    def scipy_fit():
        r = sps.linregress(index, security)
        return (r.intercept, r.slope)

    def numpy_fit():
        b, a = np.polyfit(index, security, 1)
        return (a, b)

    return Case("market_model",
                7,
                "Market model alpha and beta, 60 months",
                "exact rational least squares; error is the larger absolute "
                "error", {
                    INVESTM: investm_fit,
                    STATSMODELS: sm_fit,
                    SCIPY: scipy_fit,
                    NUMPY: numpy_fit,
                },
                lambda v: max(float(abs(Fraction(v[0]) - alpha)),
                              float(abs(Fraction(v[1]) - beta))),
                show=lambda v: f"beta {v[1]:.10f}",
                notes={
                    STATSMODELS: "OLS(...).fit(), all statistics",
                    SCIPY: "stats.linregress",
                    NUMPY: "polyfit, degree 1",
                })


def timing_case() -> Case:
    rng = np.random.default_rng(12)
    market = rng.normal(0.008, 0.045, 120)
    rf = np.full(120, 0.003)
    em = market - rf
    port = rf + 0.001 + 0.9 * em + 1.5 * em * em + rng.normal(0, 0.01, 120)
    ep = port - rf
    exact = _exact_ols([em, em * em], ep)

    def investm_fit():
        f = ch24.treynor_mazuy(port, market, rf)
        return (f.alpha, f.beta, f.timing, f.timing_t_stat)

    def sm_fit():
        fit = sm.OLS(ep, sm.add_constant(np.column_stack([em, em * em]))).fit()
        return (*fit.params, fit.tvalues[2])

    return Case(
        "treynor_mazuy",
        24,
        "Treynor-Mazuy timing regression, 120 months",
        "exact rational least squares; error is the largest coefficient "
        "error", {
            INVESTM: investm_fit,
            STATSMODELS: sm_fit,
        },
        lambda v: max(
            float(abs(Fraction(a) - b)) for a, b in zip(v[:3], exact)),
        show=lambda v: f"timing {v[2]:.6f}, t {v[3]:.6f}")


def runs_case() -> Case:
    r = np.random.default_rng(13).normal(0.0005, 0.01, 500)
    signs = [1 if x > 0 else -1 for x in r]
    n1 = mp.mpf(sum(1 for s in signs if s > 0))
    n2 = len(signs) - n1
    runs = 1 + sum(1 for a, b in zip(signs, signs[1:]) if a != b)
    n = n1 + n2
    mean = 2 * n1 * n2 / n + 1
    var = 2 * n1 * n2 * (2 * n1 * n2 - n) / (n * n * (n - 1))
    ref = (runs - mean) / mp.sqrt(var)
    return Case("runs_test",
                4,
                "Wald-Wolfowitz runs test z on the signs of 500 returns",
                "mpmath", {
                    INVESTM:
                        lambda: ch04.runs_test(r).z,
                    STATSMODELS:
                        lambda: sm_runs.runstest_1samp(
                            r, cutoff=0.0, correction=False)[0],
                },
                lambda x: float(mp.mpf(x) - ref),
                notes={
                    STATSMODELS: "runstest_1samp(cutoff=0, no continuity "
                                 "correction)"
                })


def variance_ratio_case() -> Case:
    r = np.random.default_rng(14).normal(0.0003, 0.01, 1000)
    log_prices = np.concatenate(([0.0], np.cumsum(r)))
    q = 4
    rr = [mp.mpf(x) for x in r]
    sums = [sum(rr[t - q + 1:t + 1]) for t in range(q - 1, len(rr))]

    def mp_var(x):
        m = sum(x) / len(x)
        return sum((v - m)**2 for v in x) / (len(x) - 1)

    ref = mp_var(sums) / (q * mp_var(rr))
    return Case(
        "variance_ratio",
        4, "Variance ratio VR(4) of 1,000 daily log "
        "returns, overlapping sums",
        "the book's definition in mpmath (sample variances, no "
        "bias correction)", {
            INVESTM: lambda: ch04.variance_ratio(r, q),
            ARCH: lambda: VarianceRatio(log_prices, lags=q, debiased=False).vr,
            ARCH + " (debiased)": lambda: VarianceRatio(log_prices, lags=q).vr,
        },
        lambda x: float(mp.mpf(x) - ref),
        notes={
            ARCH: "Lo-MacKinlay: drift from end points, divides by n",
            ARCH + " (debiased)": "Lo-MacKinlay with the bias correction",
        })


def autocorrelation_case() -> Case:
    x = np.random.default_rng(15).normal(0.0005, 0.01, 500)
    xs = [mp.mpf(v) for v in x]
    m = sum(xs) / len(xs)
    ref = sum((xs[t] - m) * (xs[t - 1] - m) for t in range(1, len(xs))) / sum(
        (v - m)**2 for v in xs)
    series = pd.Series(x)
    return Case("autocorrelation",
                4,
                "Lag-1 autocorrelation of 500 returns",
                "mpmath, full-sample mean and variance", {
                    INVESTM: lambda: core_stats.autocorrelation(x, 1),
                    STATSMODELS: lambda: stattools.acf(x, nlags=1)[1],
                    PANDAS: lambda: series.autocorr(1),
                },
                lambda v: float(mp.mpf(v) - ref),
                notes={
                    PANDAS: "Pearson correlation of x[1:] and x[:-1], a "
                            "different estimator"
                })


# ---------------------------------------------------------------------------
# Driver.


def all_cases() -> list[Callable[[], Any]]:
    return [
        bsm_case, greeks_case, implied_vol_case, crr_cases, norm_cdf_case,
        norm_inv_case, bond_price_case, bond_yield_case, duration_case,
        treasury_case, bootstrap_case, irr_case, tangency_case,
        _long_only_cases, implied_returns_case, style_case, market_model_case,
        timing_case, runs_case, variance_ratio_case, autocorrelation_case
    ]


def _time(fn: Callable[[], Any], repeat: int, quick: bool) -> float:
    if quick:
        return min(timeit.repeat(fn, number=1, repeat=1))
    timer = timeit.Timer(fn)
    number, _ = timer.autorange()
    number = max(1, number)
    return min(timer.repeat(repeat=repeat, number=number)) / number


def _fmt_time(seconds: float) -> str:
    for unit, scale in (("s", 1.0), ("ms", 1e-3), ("us", 1e-6)):
        if seconds >= scale:
            return f"{seconds / scale:.3g} {unit}"
    return f"{seconds / 1e-9:.3g} ns"


def _fmt_error(e: Optional[float]) -> str:
    if e is None:
        return ""
    if e == 0.0:
        return "0"
    return f"{e:.2g}"


def _versions() -> dict[str, str]:
    out = {"python": platform.python_version()}
    for dist in ("numpy", "scipy", "QuantLib", "numpy-financial", "statsmodels",
                 "pyportfolioopt", "arch", "vollib", "cvxpy", "pandas",
                 "mpmath"):
        try:
            out[dist] = metadata.version(dist)
        except metadata.PackageNotFoundError:
            out[dist] = "not installed"
    return out


def main() -> None:
    parser = argparse.ArgumentParser(
        description=__doc__.split("\n", maxsplit=1)[0])
    parser.add_argument("--filter", default="", help="substring of case names")
    parser.add_argument("--repeat", type=int, default=5)
    parser.add_argument("--quick",
                        action="store_true",
                        help="one call per library (smoke test)")
    parser.add_argument("--out", help="write results as JSON")
    args = parser.parse_args()
    warnings.filterwarnings("ignore")

    results = []
    for make in all_cases():
        made = make()
        for case in made if isinstance(made, list) else [made]:
            if args.filter not in case.name:
                continue
            print(f"\n### {case.name} (Ch. {case.chapter}): {case.title}")
            print(f"Reference: {case.reference_note}\n")
            print("| Library | Time | Value | Error | Note |")
            print("| --- | --- | --- | --- | --- |")
            rows = []
            for lib, fn in case.runners.items():
                value = fn()
                err = case.errors.get(lib)
                if err is None:
                    err = float(case.error(value))
                t = _time(fn, args.repeat, args.quick)
                note = case.notes.get(lib, "")
                print(f"| {lib} | {_fmt_time(t)} | {case.show(value)} | "
                      f"{_fmt_error(err)} | {note} |")
                rows.append({
                    "library": lib,
                    "seconds": t,
                    "value": case.show(value),
                    "error": err,
                    "note": note,
                })
            results.append({
                "case": case.name,
                "chapter": case.chapter,
                "title": case.title,
                "reference": case.reference_note,
                "results": rows,
            })
            sys.stdout.flush()

    if args.out:
        with open(args.out, "w", encoding="utf-8") as f:
            json.dump(
                {
                    "context": {
                        "host": platform.platform(),
                        "cpu_count": os.cpu_count(),
                        "date": datetime.date.today().isoformat(),
                        "versions": _versions(),
                    },
                    "cases": results,
                },
                f,
                indent=1)
            f.write("\n")


if __name__ == "__main__":
    main()
