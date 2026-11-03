"""Tests for investm.ch21_investment_companies."""

import unittest

from investm.ch21_investment_companies import funds


class FundsTest(unittest.TestCase):

    def test_nav(self):
        self.assertEqual(funds.net_asset_value(520e6, 20e6, 25e6), 20)
        self.assertAlmostEqual(funds.premium_or_discount(18, 20), -0.1)

    def test_loads(self):
        self.assertAlmostEqual(funds.offering_price(19, 0.05), 20)
        self.assertAlmostEqual(funds.fund_return(20, 21, 0.4, 0.6), 0.1)

    def test_terminal(self):
        cdsc = [0.05, 0.04, 0.03, 0.02, 0.01]
        self.assertAlmostEqual(funds.terminal_value(1000, 0, 0, 2, 0, cdsc),
                               960)
        self.assertAlmostEqual(funds.terminal_value(1000, 0, 0, 6, 0, cdsc),
                               1000)
        self.assertGreater(funds.annualized_fee_drag(0.08, 0.005, 1, 0.05),
                           funds.annualized_fee_drag(0.08, 0.005, 20, 0.05))

    def test_breakeven(self):
        y = funds.breakeven_years(funds.FundTerms(0.004, 0.0475),
                                  funds.FundTerms(0.012, 0.0), 0.08)
        self.assertLess(funds.terminal_value(1, 0.08, 0.004, y - 1, 0.0475),
                        funds.terminal_value(1, 0.08, 0.012, y - 1, 0))
        self.assertIsNone(
            funds.breakeven_years(funds.FundTerms(0.012, 0.05),
                                  funds.FundTerms(0.004, 0), 0.08))


if __name__ == "__main__":
    unittest.main()
