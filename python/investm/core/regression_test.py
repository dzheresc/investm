"""Tests for investm.core.regression."""

import unittest

import numpy as np

from investm.core import regression


class RegressionTest(unittest.TestCase):

    def test_recovers_coefficients(self):
        x1 = np.array([1, 2, 3, 4, 5, 6], dtype=float)
        x2 = np.array([2, 1, 4, 3, 6, 5], dtype=float)
        y = 0.5 + 2.0 * x1 - 1.0 * x2
        fit = regression.multiple_ols(np.column_stack([x1, x2]), y)
        np.testing.assert_allclose(fit.coefficients, [0.5, 2.0, -1.0],
                                   atol=1e-12)
        self.assertAlmostEqual(fit.r_squared, 1.0, 12)
        self.assertAlmostEqual(fit.predict([1.0, 1.0]), 1.5, 12)

    def test_collinear_returns_none(self):
        x = np.arange(5.0)
        self.assertIsNone(
            regression.multiple_ols(np.column_stack([x, 2 * x]), x + 1))

    def test_covariance_from_correlation(self):
        cov = regression.covariance_from_correlation([0.2, 0.3],
                                                     [[1, 0.5], [0.5, 1]])
        self.assertAlmostEqual(cov[0, 1], 0.03, 15)


if __name__ == "__main__":
    unittest.main()
