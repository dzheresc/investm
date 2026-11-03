"""Tests for investm.ch11_apt."""

import unittest

import numpy as np

from investm.ch11_apt import apt

_B = np.array([[0.9], [1.1], [1.5], [0.5]])


class AptTest(unittest.TestCase):

    def test_arbitrage(self):
        mu = [0.10, 0.12, 0.20, 0.07]
        x = apt.find_arbitrage_portfolio(mu, _B)
        self.assertTrue(apt.is_arbitrage_portfolio(x, _B, mu))
        self.assertGreater(x[2], 0)
        self.assertFalse(apt.is_arbitrage_portfolio([0.1, 0, 0, 0], _B, mu))

    def test_no_arbitrage_on_plane(self):
        mu = 0.04 + 0.06 * _B[:, 0]
        self.assertIsNone(apt.find_arbitrage_portfolio(mu, _B))
        p = apt.fit_apt_pricing(mu, _B)
        self.assertAlmostEqual(p.lambda0, 0.04, 12)
        self.assertAlmostEqual(p.lambdas[0], 0.06, 12)

    def test_pure_factor(self):
        b = np.array([[1.0, 0.2], [0.5, 1.0], [1.2, -0.3], [0.8, 0.7],
                      [0.3, 0.1]])
        mu = np.array(
            [apt.apt_expected_return(0.03, [0.05, 0.02], row) for row in b])
        w = apt.pure_factor_portfolio(b, 1)
        self.assertAlmostEqual(w.sum(), 1.0, 12)
        np.testing.assert_allclose(b.T @ w, [0.0, 1.0], atol=1e-12)
        self.assertAlmostEqual(float(w @ mu), 0.05, 12)


if __name__ == "__main__":
    unittest.main()
