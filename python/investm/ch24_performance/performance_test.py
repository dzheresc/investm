"""Tests for investm.ch24_performance."""

import math
import unittest

import numpy as np

from investm.ch24_performance import performance as pf

_M = [0.04, -0.03, 0.06, 0.01, -0.05, 0.07, 0.02, -0.01, 0.05, -0.02, 0.03, 0]


class PerformanceTest(unittest.TestCase):

    def test_time_vs_dollar(self):
        r = pf.period_returns_from_values(100, [120, 198], [100, 0])
        self.assertAlmostEqual(pf.time_weighted_return(r), 1.2 * 0.9 - 1)
        dw = pf.dollar_weighted_return(100, [100], 198)
        self.assertAlmostEqual(100 * (1 + dw)**2 + 100 * (1 + dw), 198, 9)

    def test_evaluate(self):
        rf = [0.002] * len(_M)
        noise = [
            0.001, -0.002, 0.0015, -0.001, 0.0005, -0.0015, 0.002, 0, -0.0005,
            0.001, -0.001, 0.0005
        ]
        p = [f + 0.003 + 1.2 * (m - f) + n for m, f, n in zip(_M, rf, noise)]
        s = pf.evaluate(p, _M, rf)
        self.assertAlmostEqual(s.beta, 1.2, delta=0.02)
        self.assertAlmostEqual(s.alpha, 0.003, delta=0.0005)
        ms = pf.evaluate(_M, _M, rf)
        self.assertAlmostEqual(ms.m_squared, 0.0)

    def test_timing(self):
        rf = [0.0] * len(_M)
        tm = pf.treynor_mazuy([0.8 * x + 3 * x * x for x in _M], _M, rf)
        self.assertAlmostEqual(tm.timing, 3.0, 8)
        hm = pf.henriksson_merton([0.5 * x + 0.7 * max(0, -x) for x in _M], _M,
                                  rf)
        self.assertAlmostEqual(hm.timing, 0.7, 8)

    def test_attribution(self):
        a = pf.brinson_attribution([0.7, 0.2, 0.1], [0.6, 0.3, 0.1],
                                   [0.055, 0.028, 0.011], [0.05, 0.025, 0.01])
        self.assertAlmostEqual(
            a.allocation.sum() + a.selection.sum() + a.interaction.sum(),
            a.total)

    def test_style(self):
        styles = np.array([[
            0.002, 0.3 * m + 0.003 * math.cos(t), 1.3 * m + 0.004 * math.sin(t)
        ] for t, m in enumerate(_M)])
        fund = styles @ np.array([0.2, 0.5, 0.3])
        fit = pf.style_analysis(fund, styles)
        self.assertAlmostEqual(fit.weights[1], 0.5, 6)
        self.assertAlmostEqual(fit.weights[2], 0.3, 6)
        self.assertAlmostEqual(fit.r_squared, 1.0, 9)


if __name__ == "__main__":
    unittest.main()
