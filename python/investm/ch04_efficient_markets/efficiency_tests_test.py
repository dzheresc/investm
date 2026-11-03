"""Tests for investm.ch04_efficient_markets."""

import math
import unittest

from investm.ch04_efficient_markets import efficiency_tests as et
from investm.ch04_efficient_markets import schedules
from investm.core import stats


class SchedulesTest(unittest.TestCase):

    def test_equilibrium(self):
        inv = [
            schedules.Investor(48, 1000, 800),
            schedules.Investor(52, 400, 900),
            schedules.Investor(55, 200, 1200),
            schedules.Investor(45, 1500, 500)
        ]
        p = schedules.equilibrium_price(inv)
        self.assertTrue(45 < p < 55)
        self.assertAlmostEqual(schedules.demand_to_buy(inv, p, 5.0),
                               schedules.supply_to_sell(inv, p, 5.0), 6)
        for i in inv:
            i.value += 2.0
        self.assertGreater(schedules.equilibrium_price(inv), p)
        self.assertIsNone(
            schedules.equilibrium_price([schedules.Investor(50, 1, 1)] * 2))

    def test_arc_elasticity(self):
        self.assertAlmostEqual(schedules.arc_elasticity(10, 100, 11, 80),
                               (-20 / 90) / (1 / 10.5), 12)


class EfficiencyTestsTest(unittest.TestCase):

    def test_runs(self):
        self.assertGreater(et.runs_test([1, -1] * 6).z, 2.0)
        clustered = et.runs_test([1] * 6 + [-1] * 6)
        self.assertLess(clustered.z, -2.0)
        self.assertEqual(clustered.runs, 2)

    def test_random_walk_passes(self):
        r = et.log_returns(et.random_walk_prices(100, 5000, 0.0, 0.01, 123))
        self.assertLess(abs(stats.autocorrelation(r, 1)), 0.05)
        self.assertLess(abs(et.variance_ratio(r, 5) - 1.0), 0.1)
        self.assertLess(abs(et.runs_test(r).z), 3.0)

    def test_variance_ratio_matches_direct(self):
        r = [0.01, -0.02, 0.015, 0.003, -0.007, 0.02, -0.01]
        sums = [sum(r[t:t + 3]) for t in range(len(r) - 2)]
        self.assertAlmostEqual(et.variance_ratio(r, 3),
                               stats.variance(sums) / (3 * stats.variance(r)),
                               14)

    def test_filter_rule(self):
        self.assertAlmostEqual(
            et.filter_rule_return([100, 102, 104, 106, 108, 110], 0.02),
            110 / 102 - 1, 12)

    def test_event_study(self):
        firms = []
        for f in range(5):
            m = [0.01 * math.sin(d + f) for d in range(30)]
            s = [
                0.001 + 1.2 * m[d] + 0.002 * math.cos(3.0 * d + f) +
                (0.05 if d == 26 else 0.0) for d in range(30)
            ]
            firms.append(et.EventFirm(s, m))
        res = et.event_study(firms, 25)
        self.assertEqual(len(res.aar), 5)
        self.assertGreater(res.aar[1], 0.04)
        self.assertLess(abs(res.aar[0]), 0.01)
        self.assertGreater(res.caar_t_stat, 3.0)


if __name__ == "__main__":
    unittest.main()
