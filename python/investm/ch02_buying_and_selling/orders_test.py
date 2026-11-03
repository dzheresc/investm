"""Tests for investm.ch02_buying_and_selling.orders."""

import unittest

from investm.ch02_buying_and_selling.orders import Order
from investm.ch02_buying_and_selling.orders import OrderType
from investm.ch02_buying_and_selling.orders import Side
from investm.ch02_buying_and_selling.orders import simulate

_PATH = [50.0, 49.2, 48.5, 47.8, 49.0, 51.5, 52.4, 50.9]


class OrdersTest(unittest.TestCase):

    def test_market_and_limit(self):
        self.assertEqual(simulate(Order(), _PATH).index, 0)
        fill = simulate(Order(Side.BUY, OrderType.LIMIT, 48.0), _PATH)
        self.assertEqual((fill.index, fill.price), (3, 47.8))
        self.assertIsNone(
            simulate(Order(Side.BUY, OrderType.LIMIT, 47.0), _PATH))

    def test_stops(self):
        stop_loss = simulate(Order(Side.SELL, OrderType.STOP, stop=48.0), _PATH)
        self.assertEqual(stop_loss.index, 3)
        stop_buy = simulate(Order(Side.BUY, OrderType.STOP, stop=51.0), _PATH)
        self.assertEqual(stop_buy.price, 51.5)

    def test_stop_limit(self):
        wide = simulate(Order(Side.BUY, OrderType.STOP_LIMIT, 51.5, 51.0),
                        _PATH)
        self.assertEqual(wide.index, 5)
        tight = simulate(Order(Side.BUY, OrderType.STOP_LIMIT, 51.0, 51.0),
                         _PATH)
        self.assertEqual(tight.index, 7)


if __name__ == "__main__":
    unittest.main()
