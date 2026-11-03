"""Tests for investm.ch25_international."""

import unittest

from investm.ch25_international import international as intl


class InternationalTest(unittest.TestCase):

    def test_currency(self):
        e = intl.currency_return(1.10, 1.21)
        self.assertAlmostEqual(e, 0.10)
        self.assertLess(intl.domestic_return(0.08, -0.10), 0)

    def test_parity(self):
        f = intl.parity_forward(1.25, 0.05, 0.02, 1)
        self.assertAlmostEqual(1.02 * f / 1.25 - 1, 0.05)
        self.assertAlmostEqual(intl.hedged_return(0.07, 0.07, 1.25, f, 0.9),
                               intl.hedged_return(0.07, 0.07, 1.25, f, 1.6))

    def test_ppp(self):
        s1 = 0.8 * (1 + intl.ppp_expected_change(0.05, 0.02))
        self.assertAlmostEqual(intl.real_exchange_rate(0.8, 100, 100),
                               intl.real_exchange_rate(s1, 102, 105))

    def test_diversification(self):
        w = intl.minimum_risk_foreign_weight(0.16, 0.20, 0.5)
        best = intl.two_market_std_dev(w, 0.16, 0.20, 0.5)
        for x in (0.0, 0.2, 0.4, 0.6, 1.0):
            self.assertGreaterEqual(intl.two_market_std_dev(x, 0.16, 0.2, 0.5),
                                    best - 1e-15)


if __name__ == "__main__":
    unittest.main()
