"""Tests for investm.ch13_fixed_income."""

import datetime
import unittest

from investm.ch05_riskless_securities import cash_flows as cf
from investm.ch13_fixed_income import fixed_income as fi

D = fi.Date
NOTE = fi.Bond(100.0, 0.05, D(2030, 11, 15))


class DayCountTest(unittest.TestCase):

    def test_conventions(self):
        self.assertEqual(
            fi.day_count_days(D(2026, 1, 31), D(2026, 3, 1),
                              fi.DayCount.THIRTY_360), 31)
        self.assertEqual(
            fi.day_count_days(D(2026, 1, 31), D(2026, 3, 1),
                              fi.DayCount.ACTUAL_ACTUAL), 29)
        self.assertEqual(fi.add_months(D(2027, 8, 31), -6), D(2027, 2, 28))
        self.assertEqual(fi.add_months(D(2028, 8, 31), -6), D(2028, 2, 29))


class FixedIncomeTest(unittest.TestCase):

    def test_period_and_accrued(self):
        p = fi.surrounding_coupons(NOTE, D(2026, 11, 3))
        self.assertEqual((p.previous, p.next, p.remaining),
                         (D(2026, 5, 15), D(2026, 11, 15), 9))
        self.assertAlmostEqual(fi.accrued_interest(NOTE, D(2026, 11, 3)),
                               2.5 * 172 / 184)

    def test_surrounding_coupons_matches_walking_back(self):
        """The direct jump agrees with stepping back from maturity."""
        for maturity in (D(2031, 8, 31), D(2030, 11, 15), D(2032, 2, 29)):
            for freq in (1, 2, 4, 12):
                bond = fi.Bond(100.0, 0.05, maturity, freq)
                step = 12 // freq
                settle = D(2026, 1, 1)
                while settle < maturity:
                    k = 0
                    nxt = maturity
                    prev = fi.add_months(maturity, -step)
                    while prev > settle:
                        k += 1
                        nxt = prev
                        prev = fi.add_months(maturity, -step * (k + 1))
                    self.assertEqual(fi.surrounding_coupons(bond, settle),
                                     fi.CouponPeriod(prev, nxt, k + 1))
                    settle += datetime.timedelta(days=3)

    def test_coupon_date_matches_cash_flows(self):
        flows = cf.coupon_bond(100.0, 0.05, 4.0, 2)
        self.assertAlmostEqual(fi.dirty_price(NOTE, D(2026, 11, 15), 0.06),
                               cf.present_value_at_yield(flows, 0.06, 2), 10)
        self.assertAlmostEqual(fi.clean_price(NOTE, D(2026, 11, 15), 0.05),
                               100.0, 10)

    def test_yield_round_trip(self):
        s = D(2026, 11, 3)
        for y in (0.01, 0.045, 0.08):
            back = fi.yield_from_clean_price(NOTE, s,
                                             fi.clean_price(NOTE, s, y))
            self.assertAlmostEqual(back, y, 10)
        self.assertEqual(fi.price_from_32nds(99, 16), 99.5)

    def test_corporate_30_360(self):
        b = fi.Bond(1000.0, 0.06, D(2031, 3, 31), 2, fi.DayCount.THIRTY_360)
        self.assertAlmostEqual(fi.accrued_interest(b, D(2026, 11, 3)),
                               30 * 33 / 180)

    def test_yield_to_worst(self):
        calls = [fi.CallDate(3.0, 103.0), fi.CallDate(5.0, 101.0)]
        w = fi.yield_to_worst(100, 0.07, 10, calls, 108.0)
        self.assertFalse(w.is_maturity)
        ytm = cf.yield_to_maturity(cf.coupon_bond(100, 0.07, 10, 2), 108, 2)
        self.assertLess(w.yield_, ytm)
        self.assertTrue(fi.yield_to_worst(100, 0.03, 10, calls, 85).is_maturity)

    def test_preferred(self):
        self.assertEqual(fi.perpetuity_value(5, 0.08), 62.5)
        self.assertEqual(fi.perpetuity_yield(5, 62.5), 0.08)


if __name__ == "__main__":
    unittest.main()
