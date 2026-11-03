"""Tests for investm.ch05_riskless_securities."""

import math
import unittest

from investm.ch05_riskless_securities import cash_flows as cf
from investm.ch05_riskless_securities import rates
from investm.ch05_riskless_securities import term_structure as ts


class RatesTest(unittest.TestCase):

    def test_fisher(self):
        self.assertAlmostEqual(rates.real_rate(0.07, 0.03), 1.07 / 1.03 - 1, 15)
        self.assertAlmostEqual(
            rates.nominal_rate(rates.real_rate(0.07, 0.03), 0.03), 0.07, 15)

    def test_compounding(self):
        self.assertAlmostEqual(rates.effective_annual_rate(0.08, 4),
                               1.02**4 - 1, 15)
        self.assertAlmostEqual(
            rates.apr_from_effective(rates.effective_annual_rate(0.08, 12), 12),
            0.08, 14)
        self.assertAlmostEqual(rates.effective_annual_rate(0.08, 10**6),
                               rates.effective_from_continuous(0.08), 7)

    def test_bank_discount(self):
        p = rates.bill_price_from_discount(10000, 0.06, 90)
        self.assertAlmostEqual(p, 9850.0, 9)
        self.assertAlmostEqual(rates.bank_discount_yield(10000, p, 90), 0.06,
                               15)
        self.assertGreater(rates.bond_equivalent_yield(10000, p, 90), 0.06)


class CashFlowsTest(unittest.TestCase):

    def test_par_bond(self):
        bond = cf.coupon_bond(1000, 0.07, 10, 2)
        self.assertEqual(len(bond), 20)
        self.assertAlmostEqual(cf.present_value_at_yield(bond, 0.07, 2), 1000,
                               9)
        self.assertAlmostEqual(cf.yield_to_maturity(bond, 1000.0, 2), 0.07, 10)

    def test_ytm_round_trip(self):
        bond = cf.coupon_bond(1000, 0.05, 7)
        for y in (0.0831, 0.75):
            self.assertAlmostEqual(
                cf.yield_to_maturity(bond, cf.present_value_at_yield(bond, y)),
                y, 10)
        self.assertAlmostEqual(
            cf.yield_to_maturity(cf.zero_coupon(1000, 3), 800.0),
            1.25**(1 / 3) - 1, 10)

    def test_irr(self):
        project = [
            cf.CashFlow(0, -1000),
            cf.CashFlow(1, 500),
            cf.CashFlow(2, 500),
            cf.CashFlow(3, 200)
        ]
        r = cf.internal_rate_of_return(project)
        self.assertAlmostEqual(cf.present_value_at_yield(project, r), 0.0, 8)
        self.assertIsNone(
            cf.internal_rate_of_return(
                [cf.CashFlow(0, 100), cf.CashFlow(1, 100)]))


class TermStructureTest(unittest.TestCase):

    def test_bootstrap_recovers_curve(self):
        truth = ts.SpotCurve([0.04, 0.045, 0.05, 0.053, 0.055])
        quotes = []
        for n in range(1, 6):
            c = 0.03 + 0.005 * n
            quotes.append(
                ts.BondQuote(100, c,
                             truth.present_value(cf.coupon_bond(100, c, n))))
        fitted = ts.bootstrap(quotes)
        for n in range(1, 6):
            self.assertAlmostEqual(fitted.spot(n), truth.spot(n), 12)
        self.assertIsNone(
            ts.bootstrap(
                [ts.BondQuote(100, 0.05, 95),
                 ts.BondQuote(100, 0.05, 3)]))

    def test_forwards(self):
        c = ts.SpotCurve([0.05, 0.06, 0.065])
        self.assertAlmostEqual(c.one_year_forward(2), 1.06**2 / 1.05 - 1, 14)
        self.assertAlmostEqual(c.forward(1, 3),
                               math.sqrt(1.065**3 / 1.05) - 1, 14)
        rebuilt = ts.SpotCurve.from_forwards(
            [c.one_year_forward(t) for t in (1, 2, 3)])
        for t in (1, 2, 3):
            self.assertAlmostEqual(rebuilt.spot(t), c.spot(t), 14)
        self.assertAlmostEqual(
            c.present_value(cf.coupon_bond(1, c.par_yield(3), 3)), 1.0, 14)

    def test_unbiased_expectations(self):
        c = ts.SpotCurve([0.04, 0.05, 0.055, 0.058])
        nxt = ts.roll_forward_one_year(c)
        for n in range(1, 5):
            self.assertAlmostEqual(ts.one_year_hpr_of_zero(c, nxt, n), 0.04, 12)

    def test_liquidity_premiums(self):
        e, prem = [0.05] * 3, [0.0, 0.003, 0.005]
        c = ts.liquidity_preference_curve(e, prem)
        for got, want in zip(ts.implied_liquidity_premiums(c, e), prem):
            self.assertAlmostEqual(got, want, 14)


if __name__ == "__main__":
    unittest.main()
