"""Tests for investm.ch22_financial_analysis."""

import math
import unittest

from investm.ch22_financial_analysis import ratios
from investm.ch22_financial_analysis import technical as ta

_F = ratios.Financials(revenue=1000,
                       ebit=150,
                       interest_expense=30,
                       income_tax=30,
                       total_assets=800,
                       equity=400,
                       current_assets=300,
                       inventory=100,
                       current_liabilities=150,
                       total_debt=250,
                       dividends=36,
                       shares=50,
                       price=27)


class RatiosTest(unittest.TestCase):

    def test_ratios(self):
        r = ratios.compute_ratios(_F)
        self.assertAlmostEqual(r.roe, 0.225)
        self.assertAlmostEqual(r.price_earnings, 15)
        self.assertAlmostEqual(r.quick_ratio, 200 / 150)

    def test_dupont(self):
        d = ratios.decompose_roe(_F)
        self.assertAlmostEqual(d.tax_burden, 0.75)
        self.assertAlmostEqual(d.roe, 0.225)


class TechnicalTest(unittest.TestCase):

    def test_moving_average(self):
        p = [10, 11, 12, 11, 10, 9, 10, 12]
        ma = ta.simple_moving_average(p, 3)
        self.assertTrue(math.isnan(ma[1]))
        self.assertAlmostEqual(ma[2], 11)
        self.assertEqual(ta.moving_average_signal(p, 3),
                         [0, 0, 1, -1, -1, -1, 1, 1])
        r = ta.moving_average_rule_returns(p, 3, 0.001)
        self.assertAlmostEqual(r[2], 11 / 12 - 1)
        self.assertEqual(r[4], 0.001)

    def test_momentum(self):
        m = ta.momentum([100, 105, 110, 99], 2)
        self.assertAlmostEqual(m[2], 0.1)
        rs = ta.relative_strength([100, 105, 110], [1000, 1050, 1050])
        self.assertAlmostEqual(rs[2], 1.1 / 1.05)


if __name__ == "__main__":
    unittest.main()
