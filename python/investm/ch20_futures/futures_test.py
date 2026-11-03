"""Tests for investm.ch20_futures."""

import math
import unittest

from investm.ch19_options import pricing as pr
from investm.ch19_options.payoffs import OptionType
from investm.ch20_futures import futures as fu


class FuturesTest(unittest.TestCase):

    def test_margin(self):
        days = fu.mark_to_market(50, [49, 47.5, 48, 51], 2, 100, 1000, 750)
        self.assertEqual(days[1].margin_call, 500)
        self.assertEqual(days[3].equity, 1700)
        self.assertAlmostEqual(sum(d.gain for d in days), 200)

    def test_carry(self):
        self.assertAlmostEqual(fu.cost_of_carry_price(100, 0.05, 0.02, 0, 1),
                               100 * math.exp(0.03))
        divs = [fu.Dividend(0.25, 1.0), fu.Dividend(1.25, 1.0)]
        self.assertAlmostEqual(fu.index_futures_price(100, 0.04, divs, 1),
                               (100 - math.exp(-0.01)) * math.exp(0.04))
        band = fu.no_arbitrage_band(100, 0.05, 0.05, 0.01, 0, 0.5)
        self.assertAlmostEqual(band.lower, band.upper)

    def test_hedge(self):
        df = [0.8, -1.1, 0.3, 1.5, -0.6, -0.9, 0.4, 1.2]
        noise = [0.05, -0.02, 0.01, -0.04, 0.03, 0.0, -0.01, 0.02]
        ds = [0.9 * f + n for f, n in zip(df, noise)]
        fit = fu.minimum_variance_hedge(ds, df)
        self.assertAlmostEqual(fu.hedge_effectiveness(ds, df, fit.beta),
                               fit.r_squared)
        self.assertEqual(fu.index_futures_contracts(2e7, 1.2, 0, 4000, 50),
                         -120)

    def test_black(self):
        f = 100 * math.exp(0.025)
        spec = pr.OptionSpec(OptionType.CALL, 100, 98, 0.05, 0.3, 0.5)
        self.assertAlmostEqual(
            fu.black_futures_option_price(OptionType.CALL, f, 98,
                                          0.05, 0.3, 0.5),
            pr.black_scholes_price(spec), 10)


if __name__ == "__main__":
    unittest.main()
