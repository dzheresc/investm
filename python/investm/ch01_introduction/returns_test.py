"""Tests for investm.ch01_introduction.returns."""

import unittest

from investm.ch01_introduction import returns


class ReturnsTest(unittest.TestCase):

    def test_holding_period_return_includes_income(self):
        self.assertAlmostEqual(returns.holding_period_return(40, 43, 1.2),
                               0.105, 15)
        with self.assertRaises(ValueError):
            returns.holding_period_return(0, 1)

    def test_simple_and_cumulative(self):
        r = returns.simple_returns([100, 110, 99])
        self.assertAlmostEqual(r[0], 0.10, 15)
        self.assertAlmostEqual(r[1], -0.10, 15)
        self.assertAlmostEqual(returns.cumulative_return(r), -0.01, 15)
        self.assertAlmostEqual(returns.annualize(0.21, 2.0), 0.10, 12)
        self.assertAlmostEqual(returns.simple_returns([50, 50], [1])[0], 0.02)

    def test_diversification_limits(self):
        f = returns.equal_weight_portfolio_std_dev
        self.assertAlmostEqual(f(1, 0.4, 0.25), 0.4, 15)
        self.assertAlmostEqual(f(100, 0.4, 0.0), 0.04, 15)
        self.assertAlmostEqual(f(50, 0.4, 1.0), 0.4, 15)
        self.assertAlmostEqual(f(1000000, 0.4, 0.25), 0.2, 6)


if __name__ == "__main__":
    unittest.main()
