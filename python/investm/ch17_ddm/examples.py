"""Worked examples for Chapter 17.

Run: python -m investm.ch17_ddm.examples
"""

from investm.ch17_ddm import ddm
from investm.core import table


def main() -> None:
    table.heading("17.3-17.4 Zero and constant growth")
    table.value("zero growth: $2.60 forever at 9.5%",
                ddm.zero_growth_value(2.60, 0.095))
    print("  constant growth, D0 = $1.80, k = 10%")
    for g in (0.0, 0.02, 0.04, 0.06, 0.08):
        print(f"    g = {100 * g:3.0f}%  V = "
              f"{ddm.constant_growth_value(1.80, g, 0.10):8.2f}")
    table.percent("implied return at P = $36, D0 = $1.80, g = 5%",
                  ddm.constant_growth_implied_return(36, 1.80, 0.05))

    table.heading("17.5 Multiple growth: four explicit years, then 4.5%")
    d = [0.90, 1.15, 1.40, 1.60]
    v = ddm.multiple_growth_value(d, 0.045, 0.105)
    table.value("value at k = 10.5%", v)
    table.value("NPV at a price of $24", v - 24)
    table.percent("implied return at $24",
                  ddm.multiple_growth_implied_return(24, d, 0.045))

    table.heading(
        "17.6 Three-phase model: 18% for 4 yrs, fading to 5% by yr 10")
    for t, x in enumerate(ddm.three_phase_dividends(1.20, 0.18, 4, 10, 0.05)):
        print(f"  year {t + 1:2d}  D = {x:6.3f}")
    for k in (0.09, 0.10, 0.11, 0.12):
        print(f"  k = {100 * k:4.1f}%  V = "
              f"{ddm.three_phase_value(1.20, 0.18, 4, 10, 0.05, k):8.2f}")

    table.heading("17.7 P/E ratios, retention and growth opportunities")
    print("  E1 = $4.00, k = 11%")
    rows = []
    for roe in (0.08, 0.11, 0.16):
        for b in (0.0, 0.4, 0.6):
            p = ddm.value_with_reinvestment(4.0, b, roe, 0.11)
            rows.append([
                100 * roe, 100 * b, 100 * ddm.sustainable_growth(roe, b), p,
                p / 4.0,
                ddm.present_value_of_growth_opportunities(p, 4.0, 0.11)
            ])
    table.table(["ROE %", "retain %", "g %", "price", "P/E", "PVGO"],
                rows,
                width=9,
                precision=2)
    table.value("forward P/E, 45% payout, g 6%, k 10%",
                ddm.forward_price_earnings(0.45, 0.06, 0.10))


if __name__ == "__main__":
    main()
