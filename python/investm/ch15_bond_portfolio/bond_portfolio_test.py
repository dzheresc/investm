"""Tests for investm.ch15_bond_portfolio."""

import unittest

from investm.ch05_riskless_securities import cash_flows as cf
from investm.ch15_bond_portfolio import bond_portfolio as bp


class HorizonTest(unittest.TestCase):

    def test_unchanged_yield(self):
        r = bp.horizon_analysis(100, 0.05, 6, 0.06, 6, 0.0, 0.06)
        self.assertAlmostEqual(r.realized_compound_yield, 0.06, 12)
        s = bp.horizon_analysis(100, 0.05, 6, 0.06, 2, 0.06, 0.06)
        self.assertAlmostEqual(s.realized_compound_yield, 0.06, 12)
        g = bp.horizon_analysis(100, 0.05, 6, 0.06, 2, 0.04, 0.06)
        self.assertGreater(g.realized_compound_yield, 0.06)


class ImmunizationTest(unittest.TestCase):

    def test_two_bonds(self):
        h = bp.immunize_two_bonds(1000, 6, 2, 10)
        self.assertAlmostEqual(
            bp.portfolio_duration([h.value1, h.value2], [2, 10]), 6)
        self.assertIsNone(bp.immunize_two_bonds(1000, 12, 2, 10))

    def test_surplus_minimum(self):
        y = 0.05
        liability = [cf.CashFlow(7.0, 1000.0)]
        pv = cf.present_value_at_yield(liability, y)
        h = bp.immunize_two_bonds(pv, 7, 3, 12)
        assets = bp.aggregate(
            [[cf.CashFlow(3.0, 1.0)], [cf.CashFlow(12.0, 1.0)]],
            [h.value1 * 1.05**3, h.value2 * 1.05**12])
        self.assertAlmostEqual(bp.surplus(assets, liability, y), 0.0, 9)
        for dy in (-0.02, 0.02):
            self.assertGreater(bp.surplus(assets, liability, y + dy), 0)

    def test_cash_match(self):
        bonds = [
            cf.coupon_bond(100, 0.04, 1),
            cf.coupon_bond(100, 0.05, 2),
            cf.coupon_bond(100, 0.06, 3)
        ]
        liabilities = [
            cf.CashFlow(1.0, 500.0),
            cf.CashFlow(2.0, 800.0),
            cf.CashFlow(3.0, 600.0)
        ]
        units = bp.cash_match(bonds, liabilities)
        for a, l in zip(bp.aggregate(bonds, units), liabilities):
            self.assertAlmostEqual(a.amount, l.amount, 9)
        self.assertIsNone(bp.cash_match(bonds, [cf.CashFlow(4.0, 1.0)]))

    def test_contingent(self):
        need = bp.required_immunized_assets(1000, 0.06, 5)
        self.assertAlmostEqual(need * 1.03**10, 1000)


class SwapTest(unittest.TestCase):

    def test_par_swap(self):
        df = [0.97, 0.94, 0.905, 0.87]
        par = bp.par_swap_rate(df, 1.0)
        self.assertAlmostEqual(bp.pay_fixed_swap_value(1e6, par, df, 1.0), 0, 6)
        self.assertGreater(bp.pay_fixed_swap_value(1e6, par - 0.005, df, 1.0),
                           0)


if __name__ == "__main__":
    unittest.main()
