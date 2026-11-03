"""Tests for investm.ch08_riskfree_borrowing_lending."""

import unittest

import numpy as np

from investm.ch07_portfolio_analysis import efficient_set as es
from investm.ch08_riskfree_borrowing_lending import riskfree as rfm
from investm.core import regression

_MU = np.array([0.08, 0.12, 0.15])
_COV = regression.covariance_from_correlation(
    [0.15, 0.22, 0.30], [[1, 0.3, 0.2], [0.3, 1, 0.4], [0.2, 0.4, 1]])


class RiskfreeTest(unittest.TestCase):

    def test_line(self):
        c = rfm.combine_with_riskfree(0.04, 0.12, 0.20, 1.5)
        self.assertAlmostEqual(c.expected_return, 0.16, 15)
        self.assertAlmostEqual(c.std_dev, 0.30, 15)

    def test_tangency_maximizes_sharpe(self):
        t = rfm.tangency_portfolio(_MU, _COV, 0.04)
        best = rfm.sharpe_ratio(t.expected_return, t.std_dev, 0.04)
        f = es.UnconstrainedFrontier.create(_MU, _COV)
        for e in np.arange(0.06, 0.25, 0.005):
            p = f.for_target_return(e)
            self.assertLessEqual(
                rfm.sharpe_ratio(p.expected_return, p.std_dev, 0.04),
                best + 1e-12)
        self.assertIsNone(rfm.tangency_portfolio(_MU, _COV, 0.5))

    def test_no_short_sales_tangency(self):
        free = rfm.tangency_portfolio(_MU, _COV, 0.04)
        np.testing.assert_allclose(rfm.tangency_portfolio_no_short_sales(
            _MU, _COV, 0.04).weights,
                                   free.weights,
                                   atol=1e-5)

    def test_no_short_sales_tangency_beats_random_portfolios(self):
        mu = np.array([0.10, 0.03, 0.14, 0.09])
        sd = np.array([0.18, 0.10, 0.28, 0.30])
        corr = np.array([[1.0, 0.2, 0.3, 0.8], [0.2, 1.0, 0.1, 0.2],
                         [0.3, 0.1, 1.0, 0.5], [0.8, 0.2, 0.5, 1.0]])
        cov = np.outer(sd, sd) * corr
        rf = 0.04
        t = rfm.tangency_portfolio_no_short_sales(mu, cov, rf)
        self.assertAlmostEqual(float(t.weights.sum()), 1.0, 12)
        self.assertTrue((t.weights >= 0).all())
        best = (t.expected_return - rf) / t.std_dev
        w = np.random.default_rng(11).exponential(size=(2000, 4))
        w /= w.sum(axis=1, keepdims=True)
        sharpe = (w @ mu - rf) / np.sqrt(np.einsum("ij,jk,ik->i", w, cov, w))
        self.assertLessEqual(float(sharpe.max()), best + 1e-9)

    def test_no_short_sales_tangency_all_below_rf(self):
        t = rfm.tangency_portfolio_no_short_sales(_MU, _COV, 0.5)
        self.assertAlmostEqual(float(t.weights.sum()), 1.0, 12)
        self.assertTrue((t.weights >= -1e-15).all())

    def test_regions(self):
        f = rfm.optimal_with_different_rates
        self.assertEqual(f(_MU, _COV, 0.03, 0.07, 0.05).region, rfm.Region.LEND)
        self.assertEqual(
            f(_MU, _COV, 0.03, 0.07, 10.0).region, rfm.Region.BORROW)
        self.assertEqual(
            f(_MU, _COV, 0.03, 0.5, 10.0).region, rfm.Region.RISKY_ONLY)


if __name__ == "__main__":
    unittest.main()
