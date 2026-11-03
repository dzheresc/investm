"""Tests for investm.ch10_factor_models."""

import unittest

import numpy as np

from investm.ch10_factor_models import factor_models as fm


def _model() -> fm.FactorModel:
    return fm.FactorModel(np.array([0.01, 0.02, 0.015]),
                          np.array([[1.0, 0.5], [0.8, -0.2], [1.5, 1.0]]),
                          np.array([0.01, 0.02, 0.03]))


class FactorModelTest(unittest.TestCase):

    def test_moments(self):
        m = _model()
        self.assertAlmostEqual(
            fm.factor_expected_returns(m, [0.05, 0.02])[0], 0.07, 15)
        fcov = np.array([[0.04, 0.005], [0.005, 0.01]])
        w = np.array([0.3, 0.3, 0.4])
        f, r = fm.decompose_portfolio_variance(m, fcov, w)
        self.assertAlmostEqual(f + r, w @ fm.factor_covariance(m, fcov) @ w, 15)

    def test_time_series(self):
        m = _model()
        rng = np.random.default_rng(5)
        f = 0.03 * rng.standard_normal((2000, 2))
        r = m.intercepts + f @ m.sensitivities.T + 0.01 * rng.standard_normal(
            (2000, 3))
        fit = fm.estimate_time_series(r, f)
        np.testing.assert_allclose(fit.sensitivities,
                                   m.sensitivities,
                                   atol=0.05)

    def test_cross_sectional(self):
        attrs = np.array([[0.5], [1.0], [1.5], [2.0]])
        f = np.array([0.02, -0.01, 0.03])
        r = 0.01 + np.outer(f, attrs[:, 0])
        v = fm.estimate_cross_sectional(r, attrs)
        np.testing.assert_allclose(v[:, 0], 0.01, atol=1e-12)
        np.testing.assert_allclose(v[:, 1], f, atol=1e-12)
        d = fm.sector_dummies([0, 0, 1, 1, 1], 2)
        v = fm.estimate_cross_sectional([[0.01, 0.03, -0.02, 0.0, 0.02]], d,
                                        False)
        np.testing.assert_allclose(v[0], [0.02, 0.0], atol=1e-14)

    def test_principal_components(self):
        rng = np.random.default_rng(1)
        b = 0.5 + 0.2 * np.arange(6)
        r = np.outer(0.03 * rng.standard_normal(3000),
                     b) + 0.002 * rng.standard_normal((3000, 6))
        pf = fm.estimate_principal_components(r, 1)
        self.assertGreater(pf.explained_fraction, 0.95)
        ratio = pf.loadings[:, 0] / b
        np.testing.assert_allclose(ratio, ratio[0], rtol=0.05)


if __name__ == "__main__":
    unittest.main()
