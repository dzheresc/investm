"""Tests for investm.ch06_portfolio_selection."""

import math
import unittest

import numpy as np

from investm.ch06_portfolio_selection import portfolio
from investm.ch06_portfolio_selection import risk_measures as rm
from investm.ch06_portfolio_selection.utility import Utility
from investm.ch06_portfolio_selection.utility import UtilityKind
from investm.ch06_portfolio_selection import utility

_W = [50.0, 150.0]
_P = [0.5, 0.5]


class UtilityTest(unittest.TestCase):

    def test_log_certainty_equivalent(self):
        u = Utility(UtilityKind.LOG)
        self.assertAlmostEqual(utility.certainty_equivalent(u, _W, _P),
                               math.sqrt(50 * 150), 10)

    def test_risk_premium_signs(self):
        self.assertAlmostEqual(
            utility.risk_premium(Utility(UtilityKind.LINEAR), _W, _P), 0, 12)
        self.assertLess(
            utility.risk_premium(Utility(UtilityKind.CONVEX), _W, _P), 0)
        self.assertGreater(
            utility.risk_premium(Utility(UtilityKind.POWER, 3.0), _W, _P),
            utility.risk_premium(Utility(UtilityKind.LOG), _W, _P))

    def test_inverse(self):
        for u in (Utility(UtilityKind.LOG), Utility(UtilityKind.POWER, 2.5),
                  Utility(UtilityKind.EXPONENTIAL,
                          0.01), Utility(UtilityKind.QUADRATIC, 0.002),
                  Utility(UtilityKind.LINEAR), Utility(UtilityKind.CONVEX)):
            self.assertAlmostEqual(u.inverse(u(120.0)), 120.0, 9)

    def test_indifference_curve(self):
        sig = [0.0, 0.1, 0.2]
        for e, s in zip(utility.indifference_curve(0.05, sig, 0.5), sig):
            self.assertAlmostEqual(utility.mean_variance_utility(e, s, 0.5),
                                   0.05, 15)


class PortfolioTest(unittest.TestCase):

    def test_holdings(self):
        w = portfolio.weights_from_holdings([150, 300, 80], [30, 22, 75])
        self.assertAlmostEqual(w[0], 4500 / 17100, 15)
        r = portfolio.portfolio_return_from_prices([150, 300, 80], [30, 22, 75],
                                                   [33, 21, 84])
        self.assertAlmostEqual(r, (4950 + 6300 + 6720) / 17100 - 1, 15)

    def test_moments(self):
        cov = [[0.04, 0.018], [0.018, 0.09]]
        self.assertAlmostEqual(portfolio.portfolio_std_dev([0.6, 0.4], cov),
                               portfolio.two_asset_std_dev(0.6, 0.2, 0.3, 0.3),
                               15)
        r = [[0.10, -0.05], [0.05, 0.05], [0.00, 0.15]]
        mu, c = portfolio.moments_from_scenarios(r, [1 / 3] * 3)
        np.testing.assert_allclose(mu, [0.05, 0.05], atol=1e-15)
        self.assertAlmostEqual(c[0, 1], -0.01 / 3, 15)


class RiskMeasuresTest(unittest.TestCase):

    def test_measures(self):
        r = [-0.10, -0.02, 0.03, 0.05, 0.08]
        self.assertAlmostEqual(rm.semi_variance(r, 0.0), 0.0104 / 5, 15)
        self.assertAlmostEqual(rm.shortfall_probability(r, 0.0), 0.4, 15)
        r = [i / 1000 - 0.05 for i in range(1, 101)]
        self.assertAlmostEqual(rm.historical_var(r, 0.95), 0.045, 12)
        self.assertAlmostEqual(rm.historical_expected_shortfall(r, 0.95), 0.047,
                               12)
        self.assertAlmostEqual(rm.normal_var(0, 0.1, 0.95), 0.1644853626951, 12)


if __name__ == "__main__":
    unittest.main()
