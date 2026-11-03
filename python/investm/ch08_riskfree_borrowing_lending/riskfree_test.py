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

    def test_regions(self):
        f = rfm.optimal_with_different_rates
        self.assertEqual(f(_MU, _COV, 0.03, 0.07, 0.05).region, rfm.Region.LEND)
        self.assertEqual(
            f(_MU, _COV, 0.03, 0.07, 10.0).region, rfm.Region.BORROW)
        self.assertEqual(
            f(_MU, _COV, 0.03, 0.5, 10.0).region, rfm.Region.RISKY_ONLY)


if __name__ == "__main__":
    unittest.main()
