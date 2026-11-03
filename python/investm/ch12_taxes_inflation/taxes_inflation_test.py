"""Tests for investm.ch12_taxes_inflation."""

import unittest

from investm.ch12_taxes_inflation import inflation as inf
from investm.ch12_taxes_inflation import taxes


class TaxesTest(unittest.TestCase):

    def test_schedule(self):
        s = taxes.TaxSchedule([
            taxes.TaxBracket(0, 0.10),
            taxes.TaxBracket(20000, 0.20),
            taxes.TaxBracket(80000, 0.35)
        ])
        self.assertAlmostEqual(s.tax(50000), 8000)
        self.assertAlmostEqual(s.tax(100000), 21000)
        self.assertEqual(s.marginal_rate(100000), 0.35)
        self.assertAlmostEqual(s.average_rate(100000), 0.21)

    def test_taxable_equivalent(self):
        t = taxes.combined_tax_rate(0.30, 0.05, True)
        self.assertAlmostEqual(t, 0.335)
        te = taxes.taxable_equivalent_yield(0.04, t)
        self.assertAlmostEqual(taxes.breakeven_tax_rate(te, 0.04), t)

    def test_deferral(self):
        a = taxes.wealth_taxed_annually(1000, 0.08, 0.3, 20)
        e = taxes.wealth_taxed_at_end(1000, 0.08, 0.3, 20)
        self.assertLess(a, e)
        g = taxes.effective_deferred_rate(0.08, 0.3, 20)
        self.assertAlmostEqual(taxes.wealth_untaxed(1000, g, 20), e, 9)


class InflationTest(unittest.TestCase):

    def test_rates(self):
        idx = [200.0, 210.0, 214.2]
        r = inf.inflation_rates(idx)
        self.assertAlmostEqual(r[0], 0.05)
        self.assertAlmostEqual(r[1], 0.02)
        self.assertAlmostEqual(inf.cumulative_inflation(idx), 0.071)

    def test_tax_and_inflation(self):
        self.assertAlmostEqual(inf.effective_tax_on_real_return(0.06, 0.3, 0),
                               0.3)
        self.assertGreater(inf.effective_tax_on_real_return(0.09, 0.3, 0.04),
                           0.3)
        real = inf.real_returns([0.10], [0.04])[0]
        self.assertAlmostEqual(inf.fisher_nominal_rate(real, 0.04), 0.10)

    def test_indexed_bond(self):
        f = inf.indexed_bond_cash_flows(1000, 0.02, 100, [103.0, 106.09])
        self.assertAlmostEqual(f[0].amount, 20.6)
        self.assertAlmostEqual(f[1].amount, 1060.9 * 1.02)
        d = inf.indexed_bond_cash_flows(1000, 0.02, 100, [98.0])
        self.assertAlmostEqual(d[0].amount, 1019.6)
        be = inf.breakeven_inflation(0.05, 0.02)
        self.assertAlmostEqual(1.02 * (1 + be), 1.05)

    def test_beta(self):
        i = [0.01, 0.03, 0.02, 0.05]
        fit = inf.inflation_beta([0.02 + x for x in i], i)
        self.assertAlmostEqual(fit.beta, 1.0)


if __name__ == "__main__":
    unittest.main()
