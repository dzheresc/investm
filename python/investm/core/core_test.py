"""Tests for investm.core."""

import math
import unittest

from investm.core import root_find
from investm.core import stats


class RootFindTest(unittest.TestCase):

    def test_solvers_find_sqrt_two(self):
        f = lambda x: x * x - 2.0
        self.assertAlmostEqual(root_find.brent(f, 0.0, 2.0), math.sqrt(2), 12)
        self.assertAlmostEqual(root_find.bisect(f, 0.0, 2.0), math.sqrt(2), 10)
        self.assertAlmostEqual(root_find.brent_expand_up(f, 0.0, 0.1),
                               math.sqrt(2), 12)

    def test_unbracketed_returns_none(self):
        f = lambda x: x * x + 1.0
        self.assertIsNone(root_find.brent(f, -1.0, 1.0))
        self.assertIsNone(root_find.bisect(f, -1.0, 1.0))

    def test_newton(self):
        root = root_find.newton(lambda x: x**3 - x - 1, lambda x: 3 * x * x - 1,
                                1.5)
        self.assertAlmostEqual(root, 1.324717957244746, 12)
        self.assertIsNone(
            root_find.newton(lambda x: x * x + 1, lambda x: 2 * x, 0.0))


class StatsTest(unittest.TestCase):

    def test_descriptive(self):
        x, y = [1, 2, 3, 4, 5], [2, 4, 6, 8, 10]
        self.assertEqual(stats.mean(x), 3.0)
        self.assertEqual(stats.variance(x), 2.5)
        self.assertEqual(stats.variance(x, 0), 2.0)
        self.assertEqual(stats.covariance(x, y), 5.0)
        self.assertAlmostEqual(stats.correlation(x, y), 1.0, 14)
        with self.assertRaises(ValueError):
            stats.covariance([1, 2, 3], [1, 2])

    def test_probability_weighted(self):
        r, p = [-0.10, 0.05, 0.20], [0.25, 0.50, 0.25]
        self.assertAlmostEqual(stats.expected(r, p), 0.05, 15)
        self.assertAlmostEqual(stats.expected_covariance(r, r, p), 0.01125, 15)

    def test_ols(self):
        x = [0.01, -0.02, 0.03, 0.00, 0.015]
        y = [0.002 + 1.3 * v for v in x]
        fit = stats.ols(x, y)
        self.assertAlmostEqual(fit.alpha, 0.002, 13)
        self.assertAlmostEqual(fit.beta, 1.3, 11)
        self.assertAlmostEqual(fit.r_squared, 1.0, 11)

    def test_normal_and_geometric(self):
        self.assertEqual(stats.norm_cdf(0.0), 0.5)
        self.assertAlmostEqual(stats.norm_cdf(1.96), 0.9750021048517795, 12)
        self.assertAlmostEqual(stats.geometric_mean_return([0.1, -0.1]),
                               math.sqrt(0.99) - 1, 15)
        self.assertLess(stats.autocorrelation([1, -1] * 4, 1), -0.8)


if __name__ == "__main__":
    unittest.main()
