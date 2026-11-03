"""Tests for investm.ch23_investment_management."""

import unittest

from investm.ch23_investment_management import management as mg


class ManagementTest(unittest.TestCase):

    def test_tolerance(self):
        tau = mg.implied_risk_tolerance(0.6, 0.09, 0.18, 0.03)
        self.assertAlmostEqual(mg.optimal_stock_weight(tau, 0.09, 0.18, 0.03),
                               0.6)

    def test_buy_and_hold(self):
        p = mg.buy_and_hold([0.10, -0.20, 0.15], 0.01, 0.6, 100)
        self.assertAlmostEqual(p.wealth[-1],
                               60 * 1.1 * 0.8 * 1.15 + 40 * 1.01**3)

    def test_constant_mix(self):
        r = [0.25, -0.20, 0.25, -0.20]
        self.assertGreater(
            mg.constant_mix(r, 0, 0.5).wealth[-1],
            mg.buy_and_hold(r, 0, 0.5).wealth[-1])
        up = [0.2] * 4
        self.assertLess(
            mg.constant_mix(up, 0, 0.5).wealth[-1],
            mg.buy_and_hold(up, 0, 0.5).wealth[-1])

    def test_cppi_floor(self):
        p = mg.cppi([-0.10, -0.15, -0.20, -0.25, -0.10], 0.01, 3, 0.8)
        self.assertGreater(p.wealth[-1], 0.8 * 1.01**5 - 1e-12)
        self.assertAlmostEqual(p.risky_weight[0], 0.6)


if __name__ == "__main__":
    unittest.main()
