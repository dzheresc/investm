"""Tests for investm.ch07_portfolio_analysis."""

import unittest

import numpy as np

from investm.ch07_portfolio_analysis import efficient_set as es
from investm.ch07_portfolio_analysis import market_model as mm
from investm.core import regression

_MU = np.array([0.08, 0.12, 0.15])
_COV = regression.covariance_from_correlation(
    [0.15, 0.22, 0.30], [[1, 0.3, 0.2], [0.3, 1, 0.4], [0.2, 0.4, 1]])


class EfficientSetTest(unittest.TestCase):

    def test_frontier(self):
        f = es.UnconstrainedFrontier.create(_MU, _COV)
        mv = f.minimum_variance()
        self.assertAlmostEqual(mv.weights.sum(), 1.0, 12)
        self.assertAlmostEqual(mv.std_dev**2, 1 / f.a, 12)
        for e in (0.09, 0.12, 0.2):
            p = f.for_target_return(e)
            self.assertAlmostEqual(p.expected_return, e, 12)
            self.assertAlmostEqual(p.std_dev**2, f.variance_at(e), 12)
        opt = f.optimal(0.5)
        np.testing.assert_allclose(opt.weights,
                                   f.for_target_return(
                                       opt.expected_return).weights,
                                   atol=1e-12)
        self.assertIsNone(es.UnconstrainedFrontier.create([0.1] * 3, _COV))

    def test_simplex(self):
        np.testing.assert_allclose(es.project_onto_simplex([0.5, 0.9, -1.0]),
                                   [0.3, 0.7, 0.0],
                                   atol=1e-15)

    def test_affine_simplex_projection(self):
        rng = np.random.default_rng(5)
        for _ in range(20):
            v = rng.normal(size=6)
            np.testing.assert_allclose(es.project_onto_affine_simplex(
                v, np.ones(6)),
                                       es.project_onto_simplex(v),
                                       atol=1e-14)
        a = np.array([0.04, -0.02, 0.0, 0.10])
        for v in ([1.0, 2.0, -1.0, 0.5], [0.0] * 4, [-5.0, 3.0, 2.0, -5.0]):
            v = np.asarray(v)
            z = es.project_onto_affine_simplex(v, a)
            self.assertTrue((z >= 0).all())
            self.assertAlmostEqual(float(a @ z), 1.0, 12)
            k = int(np.nonzero((z > 0) & (a != 0))[0][0])
            theta = (v[k] - z[k]) / a[k]
            slack = theta * a - (v - z)
            np.testing.assert_allclose(slack[z > 0], 0.0, atol=1e-12)
            self.assertTrue((slack[z == 0] >= -1e-12).all())

    def test_efficient_set_warm_start(self):
        taus = [0.0, 0.1, 0.3, 1.0, 5.0]
        for p, tau in zip(es.efficient_set_no_short_sales(_MU, _COV, taus),
                          taus):
            np.testing.assert_allclose(p.weights,
                                       es.optimal_no_short_sales(
                                           _MU, _COV, tau).weights,
                                       atol=1e-9)

    def test_no_short_sales(self):
        free = es.UnconstrainedFrontier.create(_MU, _COV).optimal(0.3)
        self.assertTrue((free.weights > 0).all())
        np.testing.assert_allclose(es.optimal_no_short_sales(_MU, _COV,
                                                             0.3).weights,
                                   free.weights,
                                   atol=1e-8)
        top = es.optimal_no_short_sales(_MU, _COV, 100.0)
        self.assertAlmostEqual(top.weights[2], 1.0, 9)

    def test_kkt(self):
        tau = 3.0
        p = es.optimal_no_short_sales(_MU, _COV, tau)
        g = 2 * _COV @ p.weights - tau * _MU
        active = p.weights > 1e-9
        nu = g[active].min()
        np.testing.assert_allclose(g[active], nu, atol=1e-7)
        self.assertTrue((g[~active] >= nu - 1e-7).all())

    def test_two_asset(self):
        self.assertAlmostEqual(es.two_asset_min_variance_weight(0.2, 0.4, -1),
                               0.4 / 0.6, 15)


class MarketModelTest(unittest.TestCase):

    def test_decomposition(self):
        models = [mm.MarketModel(0, 0.8, 0.02), mm.MarketModel(0, 1.2, 0.03)]
        r = mm.portfolio_risk([0.5, 0.5], models, 0.04)
        self.assertAlmostEqual(r.beta, 1.0, 15)
        cov = mm.market_model_covariance(models, 0.04)
        w = np.array([0.5, 0.5])
        self.assertAlmostEqual(w @ cov @ w, r.total_variance, 15)

    def test_random_diversification(self):
        self.assertAlmostEqual(
            mm.random_diversification_variance(1, 0.16, 0.04), 0.16)
        v, c = mm.average_variance_and_covariance([[0.04, 0.01, 0.02],
                                                   [0.01, 0.09, 0.03],
                                                   [0.02, 0.03, 0.16]])
        self.assertAlmostEqual(v, 0.29 / 3, 15)
        self.assertAlmostEqual(c, 0.02, 15)


if __name__ == "__main__":
    unittest.main()
