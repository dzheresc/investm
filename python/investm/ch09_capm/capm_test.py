"""Tests for investm.ch09_capm."""

import unittest

import numpy as np

from investm.ch07_portfolio_analysis import efficient_set as es
from investm.ch08_riskfree_borrowing_lending import riskfree as rfm
from investm.ch09_capm import capm
from investm.core import regression

_MU = np.array([0.08, 0.12, 0.15])
_COV = regression.covariance_from_correlation(
    [0.15, 0.22, 0.30], [[1, 0.3, 0.2], [0.3, 1, 0.4], [0.2, 0.4, 1]])


class CapmTest(unittest.TestCase):

    def test_sml_holds_for_tangency_market(self):
        m = rfm.tangency_portfolio(_MU, _COV, 0.04)
        b = capm.betas(_COV, m.weights)
        np.testing.assert_allclose(_MU,
                                   0.04 + b * (m.expected_return - 0.04),
                                   atol=1e-12)
        self.assertAlmostEqual(float(m.weights @ b), 1.0, 12)

    def test_implied_returns(self):
        w = [0.5, 0.3, 0.2]
        mu = capm.implied_expected_returns(_COV, w, 0.03, 0.05)
        t = rfm.tangency_portfolio(mu, _COV, 0.03)
        np.testing.assert_allclose(t.weights, w, atol=1e-10)

    def test_zero_beta(self):
        f = es.UnconstrainedFrontier.create(_MU, _COV)
        ez = capm.zero_beta_return(f.a, f.b, f.c, 0.13)
        m = f.for_target_return(0.13)
        z = f.for_target_return(ez)
        self.assertAlmostEqual(float(m.weights @ _COV @ z.weights), 0.0, 12)

    def test_partition(self):
        mkt, own = capm.partition_risk(0.09, 1.2, 0.04)
        self.assertAlmostEqual(mkt, 0.0576, 15)
        self.assertAlmostEqual(own, 0.0324, 15)


if __name__ == "__main__":
    unittest.main()
