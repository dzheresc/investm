"""Tests for investm.ch19_options."""

import dataclasses
import itertools
import math
import unittest

from investm.ch19_options import payoffs as po
from investm.ch19_options import pricing as pr

CALL = po.OptionType.CALL
PUT = po.OptionType.PUT


def _spec(t):
    return pr.OptionSpec(t, 100.0, 95.0, 0.04, 0.25, 0.75)


def _tree(spec, steps):
    """Plain backward induction, the definition of the European price."""
    dt = spec.years / steps
    u = math.exp(spec.volatility * math.sqrt(dt))
    p = (math.exp((spec.rate - spec.dividend_yield) * dt) - 1 / u) / (u - 1 / u)
    disc = math.exp(-spec.rate * dt)
    v = [
        po.payoff(spec.option_type, spec.strike, spec.spot * u**(2 * j - steps))
        for j in range(steps + 1)
    ]
    for i in range(steps - 1, -1, -1):
        v = [disc * (p * v[j + 1] + (1 - p) * v[j]) for j in range(i + 1)]
    return v[0]


class PayoffTest(unittest.TestCase):

    def test_strategies(self):
        straddle = [
            po.Leg(po.Instrument.CALL, 100, 1, 6),
            po.Leg(po.Instrument.PUT, 100, 1, 5)
        ]
        self.assertEqual(po.strategy_profit(straddle, 100), -11)
        self.assertEqual(po.strategy_profit(straddle, 89), 0)
        fid = [
            po.Leg(po.Instrument.CALL, 40, 1, 0),
            po.Leg(po.Instrument.BOND, 40, 1, 0)
        ]
        prot = [
            po.Leg(po.Instrument.PUT, 40, 1, 0),
            po.Leg(po.Instrument.STOCK, 0, 1, 0)
        ]
        for s in range(20, 61, 5):
            self.assertEqual(po.strategy_payoff(fid, s),
                             po.strategy_payoff(prot, s))

    def test_parity(self):
        p = po.parity_put_price(5, 42, 40, 0.05, 0.5)
        self.assertAlmostEqual(po.parity_call_price(p, 42, 40, 0.05, 0.5), 5)
        self.assertAlmostEqual(po.parity_deviation(5, p, 42, 40, 0.05, 0.5), 0)


class PricingTest(unittest.TestCase):

    def test_one_period(self):
        r = pr.one_period_binomial(CALL, 50, 50, 1.2, 0.8, 0.05)
        self.assertAlmostEqual(r.shares, 0.5)
        self.assertAlmostEqual(r.shares * 60 + r.bond * 1.05, 10)

    def test_binomial_converges(self):
        for t in (CALL, PUT):
            s = _spec(t)
            self.assertAlmostEqual(pr.binomial_price(s, 2000, False),
                                   pr.black_scholes_price(s),
                                   delta=5e-3)

    def test_european_matches_backward_induction(self):
        for t in (CALL, PUT):
            for strike in (60.0, 95.0, 140.0):
                for steps in (1, 2, 7, 50, 501):
                    s = dataclasses.replace(_spec(t),
                                            strike=strike,
                                            dividend_yield=0.015)
                    self.assertAlmostEqual(pr.binomial_price(s, steps, False),
                                           _tree(s, steps),
                                           delta=1e-11)

    def test_american(self):
        c = _spec(CALL)
        self.assertAlmostEqual(pr.binomial_price(c, 300, True),
                               pr.binomial_price(c, 300, False), 10)
        p = dataclasses.replace(_spec(PUT), rate=0.08)
        self.assertGreater(pr.binomial_price(p, 300, True),
                           pr.binomial_price(p, 300, False))

    def test_greeks(self):
        c = dataclasses.replace(_spec(CALL), dividend_yield=0.02)
        g = pr.black_scholes_greeks(c)
        h = 1e-4

        def bump(field, dx):
            return pr.black_scholes_price(
                dataclasses.replace(c, **{field: getattr(c, field) + dx}))

        self.assertAlmostEqual(g.delta,
                               (bump("spot", h) - bump("spot", -h)) / (2 * h),
                               6)
        self.assertAlmostEqual(
            g.vega, (bump("volatility", h) - bump("volatility", -h)) / (2 * h),
            4)
        self.assertAlmostEqual(
            g.theta, -(bump("years", h) - bump("years", -h)) / (2 * h), 4)
        p = dataclasses.replace(c, option_type=PUT)
        self.assertAlmostEqual(
            pr.black_scholes_price(c) - pr.black_scholes_price(p),
            100 * math.exp(-0.015) - 95 * math.exp(-0.03))

    def test_implied_vol(self):
        s = _spec(PUT)
        price = pr.black_scholes_price(s)
        self.assertAlmostEqual(pr.implied_volatility(s, price), 0.25, 9)
        itm = dataclasses.replace(s, strike=120.0)
        self.assertIsNone(pr.implied_volatility(itm, 10.0))

    def test_implied_vol_round_trip_over_grid(self):
        grid = itertools.product(
            (CALL, PUT), (40.0, 70.0, 95.0, 100.0, 105.0, 140.0, 250.0),
            (0.02, 0.25, 1.0, 5.0, 30.0), (0.03, 0.1, 0.25, 0.6, 1.5),
            (0.0, 0.03))
        for t, strike, years, vol, q in grid:
            spec = pr.OptionSpec(t, 100.0, strike, 0.04, vol, years, q)
            # Deep out of the money the price says nothing about volatility
            # at double precision.
            if pr.black_scholes_greeks(spec).vega < 1e-6:
                continue
            implied = pr.implied_volatility(
                dataclasses.replace(spec, volatility=0.2),
                pr.black_scholes_price(spec))
            self.assertIsNotNone(implied, spec)
            self.assertAlmostEqual(implied,
                                   vol,
                                   delta=1e-9 * max(1.0, 1e-3 / years),
                                   msg=str(spec))

    def test_price_matches_greeks_price(self):
        for t in (CALL, PUT):
            spec = dataclasses.replace(_spec(t), dividend_yield=0.02)
            self.assertAlmostEqual(pr.black_scholes_price(spec),
                                   pr.black_scholes_greeks(spec).price,
                                   delta=1e-13)

    def test_warrant(self):
        s = _spec(CALL)
        self.assertAlmostEqual(pr.warrant_value(s, 1e6, 2.5e5),
                               0.8 * pr.black_scholes_price(s))


if __name__ == "__main__":
    unittest.main()
