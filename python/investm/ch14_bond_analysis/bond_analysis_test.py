"""Tests for investm.ch14_bond_analysis."""

import unittest

from investm.ch05_riskless_securities import cash_flows as cf
from investm.ch14_bond_analysis import credit
from investm.ch14_bond_analysis import duration as dur


class DurationTest(unittest.TestCase):

    def test_zero(self):
        self.assertAlmostEqual(
            dur.macaulay_duration(cf.zero_coupon(100, 7), 0.05), 7.0)

    def test_matches_finite_differences(self):
        flows = cf.coupon_bond(100, 0.06, 10, 2)

        def price(r):
            return cf.present_value_at_yield(flows, r, 2)

        self.assertAlmostEqual(dur.modified_duration(flows, 0.07, 2),
                               dur.effective_duration(price, 0.07), 5)
        self.assertAlmostEqual(dur.convexity(flows, 0.07, 2),
                               dur.effective_convexity(price, 0.07, 1e-3), 2)

    def test_convexity_helps(self):
        flows = cf.coupon_bond(100, 0.05, 20)
        p = cf.present_value_at_yield(flows, 0.05)
        d = dur.modified_duration(flows, 0.05)
        c = dur.convexity(flows, 0.05)
        for dy in (-0.02, 0.02):
            actual = cf.present_value_at_yield(flows, 0.05 + dy) - p
            self.assertLess(abs(actual - dur.approx_price_change(p, d, c, dy)),
                            abs(actual - dur.approx_price_change(p, d, 0, dy)))

    def test_perpetuity(self):
        flows = cf.coupon_bond(100, 0.08, 400)
        self.assertAlmostEqual(dur.macaulay_duration(flows, 0.08),
                               dur.perpetuity_duration(0.08), 6)


class CreditTest(unittest.TestCase):

    def test_expected_flows(self):
        f = credit.expected_cash_flows(100, 0.08, 1, 0.05, 0.4)
        self.assertAlmostEqual(f[0].amount, 0.95 * 108 + 0.05 * 40)

    def test_decomposition(self):
        d = credit.decompose_yield(100, 0.08, 5, 97, 0.02, 0.45, 0.04)
        self.assertGreater(d.default_premium, 0)
        self.assertAlmostEqual(d.promised_yield - d.default_premium,
                               d.expected_yield, 14)

    def test_implied_default(self):
        p = credit.implied_default_probability(0.07, 0.04, 0.4)
        self.assertAlmostEqual(1.07 * (1 - 0.6 * p), 1.04)

    def test_discriminant(self):
        healthy = [[8.0, 0.30], [6.5, 0.35], [9.2, 0.25], [7.1, 0.40]]
        failed = [[1.2, 0.75], [2.0, 0.65], [0.8, 0.80], [1.5, 0.70]]
        lda = credit.LinearDiscriminant.fit(healthy, failed)
        self.assertTrue(all(lda.is_group_a(x) for x in healthy))
        self.assertFalse(any(lda.is_group_a(x) for x in failed))


if __name__ == "__main__":
    unittest.main()
