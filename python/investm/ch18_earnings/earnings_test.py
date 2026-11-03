"""Tests for investm.ch18_earnings."""

import unittest

from investm.ch18_earnings import earnings as ea


class EarningsTest(unittest.TestCase):

    def test_lintner(self):
        truth = ea.LintnerModel(0.02, 0.35, 0.50, 1.0)
        e = [2.0, 2.4, 2.1, 2.9, 3.3, 3.0, 3.8, 4.1]
        d = [0.9]
        for x in e[1:]:
            d.append(truth.next_dividend(d[-1], x))
        fit = ea.fit_lintner(e, d)
        self.assertAlmostEqual(fit.adjustment, 0.35, 10)
        self.assertAlmostEqual(fit.target_payout, 0.50, 10)

    def test_forecasts(self):
        self.assertAlmostEqual(ea.random_walk_forecast([1.0, 1.2, 1.1, 1.6]),
                               1.8)
        q = [0.5, 0.8, 0.6, 1.0, 0.54, 0.84, 0.64, 1.04]
        self.assertAlmostEqual(ea.seasonal_random_walk_forecast(q), 0.58)

    def test_sue(self):
        q = [0.50, 0.80, 0.60, 1.00, 0.53, 0.85, 0.63, 1.03, 0.58, 0.87]
        f = ea.seasonal_random_walk_forecast(q)
        self.assertAlmostEqual(ea.standardized_unexpected_earnings(q, f), 0)
        self.assertGreater(ea.standardized_unexpected_earnings(q, f + 0.05), 2)

    def test_groups(self):
        g = ea.sort_into_groups([0.3, -1.2, 2.5, 0.0, -0.4, 1.1], 3)
        self.assertEqual(g, [1, 0, 2, 1, 0, 2])
        m = ea.group_means([0.01, -0.02, 0.04, 0.0, -0.01, 0.03], g, 3)
        self.assertAlmostEqual(m[0], -0.015)
        self.assertAlmostEqual(m[2], 0.035)


if __name__ == "__main__":
    unittest.main()
