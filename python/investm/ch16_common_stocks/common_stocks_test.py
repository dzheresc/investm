"""Tests for investm.ch16_common_stocks."""

import unittest

from investm.ch16_common_stocks import betas
from investm.ch16_common_stocks import corporate as corp


class CorporateTest(unittest.TestCase):

    def test_voting(self):
        self.assertEqual(corp.cumulative_voting_shares_needed(1000, 5, 1), 167)
        self.assertEqual(corp.cumulative_voting_shares_needed(1000, 5, 3), 501)
        self.assertEqual(corp.majority_voting_shares_needed(1000), 501)

    def test_splits(self):
        adj = corp.split_adjust_prices([100, 104, 52.5, 53], [1, 1, 2, 1])
        self.assertEqual(adj, [50, 52, 52.5, 53])
        self.assertAlmostEqual(
            corp.price_after_split(55,
                                   corp.split_ratio_from_stock_dividend(0.1)),
            50)

    def test_rights(self):
        terms = corp.RightsTerms(4, 40.0)
        ex = corp.ex_rights_price(50, terms)
        self.assertEqual(ex, 48)
        self.assertEqual(corp.right_value_cum(50, terms), 2)
        self.assertEqual(corp.right_value_ex(ex, terms), 2)


class BetasTest(unittest.TestCase):

    def test_rolling(self):
        m = [0.02, -0.01, 0.03, 0.0, -0.02, 0.04, 0.01, -0.03, 0.02, 0.05]
        s = [0.001 + (1.5 if t < 5 else 0.8) * x for t, x in enumerate(m)]
        r = betas.rolling_betas(s, m, 5)
        self.assertAlmostEqual(r[0], 1.5)
        self.assertAlmostEqual(r[1], 0.8)

    def test_adjustments(self):
        self.assertAlmostEqual(betas.shrink_beta(1.6), 1.4)
        self.assertAlmostEqual(betas.vasicek_beta(1.6, 0.3, 1.0, 0.3), 1.3)
        self.assertLess(
            betas.beta_persistence([0.5, 0.8, 1.0, 1.3, 1.7],
                                   [0.7, 0.9, 1.0, 1.2, 1.4]).beta, 1)

    def test_leverage(self):
        bl = betas.levered_beta(0.9, 0.5, 0.25)
        self.assertAlmostEqual(bl, 0.9 * 1.375)
        self.assertAlmostEqual(betas.unlevered_beta(bl, 0.5, 0.25), 0.9)


if __name__ == "__main__":
    unittest.main()
