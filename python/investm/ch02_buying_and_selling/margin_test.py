"""Tests for investm.ch02_buying_and_selling.margin."""

import unittest

from investm.ch02_buying_and_selling import margin


class MarginTest(unittest.TestCase):

    def test_margin_purchase(self):
        mp = margin.MarginPurchase(100, 100.0, 0.60)
        self.assertEqual(mp.loan, 4000.0)
        call = mp.margin_call_price(0.30)
        self.assertAlmostEqual(call, 4000.0 / 70.0, 12)
        self.assertAlmostEqual(mp.actual_margin(call), 0.30, 12)
        self.assertAlmostEqual(mp.cash_to_restore(50.0, 0.6), 2000.0, 9)
        self.assertAlmostEqual(mp.return_on_equity(110.0, 0.0, 0.10), 0.10, 12)

    def test_short_sale(self):
        ss = margin.ShortSale(100, 100.0, 0.50)
        call = ss.margin_call_price(0.30)
        self.assertAlmostEqual(call, 15000.0 / 130.0, 12)
        self.assertAlmostEqual(ss.actual_margin(call), 0.30, 12)
        self.assertAlmostEqual(ss.return_on_equity(80.0, 1.0), 0.38, 12)

    def test_hedged_account(self):
        account = margin.Account(
            [margin.Position(100, 50.0),
             margin.Position(-100, 50.0)], 5000.0)
        self.assertEqual(account.equity, 5000.0)
        self.assertEqual(account.actual_margin(), 0.5)
        account.positions = [
            margin.Position(100, 40.0),
            margin.Position(-100, 40.0)
        ]
        self.assertEqual(account.equity, 5000.0)


if __name__ == "__main__":
    unittest.main()
