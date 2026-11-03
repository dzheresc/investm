"""Tests for investm.ch17_ddm."""

import unittest

from investm.ch17_ddm import ddm


class DdmTest(unittest.TestCase):

    def test_simple(self):
        self.assertAlmostEqual(ddm.zero_growth_value(3, 0.12), 25)
        self.assertAlmostEqual(ddm.constant_growth_value(2, 0.05, 0.10), 42)
        self.assertAlmostEqual(ddm.constant_growth_implied_return(42, 2, 0.05),
                               0.10)

    def test_multiple_growth(self):
        d = [1.5 * 1.04**t for t in range(1, 7)]
        self.assertAlmostEqual(ddm.multiple_growth_value(d, 0.04, 0.09),
                               ddm.constant_growth_value(1.5, 0.04, 0.09), 10)
        v = ddm.multiple_growth_value([1.0, 1.25, 1.5, 1.7], 0.05, 0.11)
        self.assertAlmostEqual(
            ddm.multiple_growth_implied_return(v, [1.0, 1.25, 1.5, 1.7], 0.05),
            0.11, 10)

    def test_three_phase(self):
        d = ddm.three_phase_dividends(1.0, 0.20, 2, 5, 0.05)
        self.assertAlmostEqual(d[4], 1.44 * 1.15 * 1.10 * 1.05)
        self.assertAlmostEqual(ddm.three_phase_value(1, 0.05, 2, 5, 0.05, 0.1),
                               ddm.constant_growth_value(1, 0.05, 0.1), 10)

    def test_growth(self):
        base = ddm.value_with_reinvestment(5, 0, 0.15, 0.12)
        self.assertGreater(ddm.value_with_reinvestment(5, 0.5, 0.15, 0.12),
                           base)
        self.assertLess(ddm.value_with_reinvestment(5, 0.5, 0.09, 0.12), base)
        self.assertAlmostEqual(ddm.trailing_price_earnings(0.4, 0.06, 0.10),
                               10.6)


if __name__ == "__main__":
    unittest.main()
