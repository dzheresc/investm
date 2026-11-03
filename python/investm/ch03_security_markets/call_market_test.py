"""Tests for investm.ch03_security_markets."""

import unittest

from investm.ch03_security_markets import call_market
from investm.ch03_security_markets import costs
from investm.ch03_security_markets.call_market import LimitOrder

_BOOK = [
    LimitOrder(True, 30.50, 300),
    LimitOrder(True, 30.25, 500),
    LimitOrder(True, 30.00, 400),
    LimitOrder(True, 29.75, 600),
    LimitOrder(False, 29.50, 200),
    LimitOrder(False, 29.75, 300),
    LimitOrder(False, 30.00, 500),
    LimitOrder(False, 30.25, 700),
]


class CallMarketTest(unittest.TestCase):

    def test_clears_at_max_volume(self):
        r = call_market.clear_call_market(_BOOK)
        self.assertEqual((r.price, r.volume, r.imbalance), (30.0, 1000, 200))

    def test_non_crossing_book(self):
        r = call_market.clear_call_market(
            [LimitOrder(True, 10.0, 100),
             LimitOrder(False, 11.0, 100)])
        self.assertEqual((r.price, r.volume), (10.5, 0))


class CostsTest(unittest.TestCase):

    def test_commission_and_spread(self):
        c = costs.CommissionSchedule(10.0, 0.01, 0.0, 15.0)
        self.assertEqual(c.commission(100, 50), 15.0)
        self.assertAlmostEqual(c.commission(1000, 50), 20.0)
        q = costs.Quote(9.95, 10.05)
        self.assertAlmostEqual(q.relative_spread, 0.01, 12)
        self.assertAlmostEqual(costs.effective_half_spread(10.02, q), 0.002, 12)
        self.assertAlmostEqual(costs.sqrt_price_impact(10000, 1e6, 0.02), 0.002,
                               15)
        self.assertAlmostEqual(
            costs.one_way_cost(1000, 10.0, q,
                               costs.CommissionSchedule(0, 0, 0.001), 0.002),
            0.008, 12)


if __name__ == "__main__":
    unittest.main()
