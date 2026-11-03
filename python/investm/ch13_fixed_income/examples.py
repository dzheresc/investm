"""Worked examples for Chapter 13.

Run: python -m investm.ch13_fixed_income.examples
"""

from investm.ch13_fixed_income import fixed_income as fi
from investm.core import table

D = fi.Date


def main() -> None:
    table.heading("13.3 A Treasury note between coupon dates")
    note = fi.Bond(100.0, 0.0425, D(2034, 2, 15))
    settle = D(2026, 11, 3)
    p = fi.surrounding_coupons(note, settle)
    print("  4.25% note due 2034-02-15, settling 2026-11-03")
    print(f"  last coupon {p.previous}, next {p.next}, {p.remaining} to go")
    quoted = fi.price_from_32nds(98, 11)
    accrued = fi.accrued_interest(note, settle)
    table.value("quoted price 98-11", quoted)
    table.value("accrued interest", accrued)
    table.value("invoice (dirty) price", quoted + accrued)
    table.percent("yield to maturity (bond-equivalent)",
                  fi.yield_from_clean_price(note, settle, quoted))
    print("\n  clean vs dirty price at 4.5% through a coupon date:")
    for day in (10, 13, 14, 15, 16, 17):
        s = D(2027, 2, day)
        print(f"    {s}  clean {fi.clean_price(note, s, 0.045):8.4f}  "
              f"dirty {fi.dirty_price(note, s, 0.045):8.4f}")

    table.heading("13.5 A corporate bond on a 30/360 basis")
    corp = fi.Bond(1000.0, 0.0575, D(2033, 6, 30), 2, fi.DayCount.THIRTY_360)
    table.value("accrued per $1000", fi.accrued_interest(corp, settle))
    table.value("clean price at 6.40%", fi.clean_price(corp, settle, 0.064))

    table.heading("13.6 Callable bonds: yield-to-call and yield-to-worst")
    calls = [
        fi.CallDate(2.0, 104.0),
        fi.CallDate(4.0, 102.0),
        fi.CallDate(6.0, 100.0)
    ]
    print("  6.5% 12-year bond, call schedule 104 (yr 2), 102 (yr 4), "
          "100 (yr 6)")
    for price in (92.0, 100.0, 106.0, 112.0):
        w = fi.yield_to_worst(100.0, 0.065, 12.0, calls, price)
        kind = "maturity" if w.is_maturity else "call"
        print(f"  {price:8.2f} {100 * w.yield_:9.3f}% {w.horizon:9.0f} yr "
              f"({kind})")

    table.heading("13.8 Preferred stock")
    table.value("value of $4.40 dividend at 6.5%",
                fi.perpetuity_value(4.40, 0.065))
    table.percent("yield at a price of $58", fi.perpetuity_yield(4.40, 58.0))


if __name__ == "__main__":
    main()
