"""Worked examples for Chapter 15.

Run: python -m investm.ch15_bond_portfolio.examples
"""

from investm.ch05_riskless_securities import cash_flows as cf
from investm.ch14_bond_analysis import duration as dur
from investm.ch15_bond_portfolio import bond_portfolio as bp
from investm.core import table


def main() -> None:
    table.heading("15.2.1 Horizon analysis: 5.25% 10-year bought at 5.60%")
    print("  3-year horizon, coupons reinvested at 4.5%")
    rows = []
    for end in (0.045, 0.052, 0.056, 0.060, 0.067):
        r = bp.horizon_analysis(100, 0.0525, 10, 0.056, 3, end, 0.045)
        rows.append([
            100 * end, r.sale_price, r.coupon_income, r.interest_on_interest,
            100 * r.total_return, 100 * r.realized_compound_yield
        ])
    table.table(["end yld %", "sale", "coupons", "int/int", "total %", "RCY %"],
                rows,
                width=10,
                precision=3)

    table.heading("15.2.2 A substitution swap with a one-year workout")
    a = bp.horizon_analysis(100, 0.05, 8, 0.055, 1, 0.055, 0.05)
    b = bp.horizon_analysis(100, 0.05, 8, 0.0575, 1, 0.055, 0.05)
    table.percent("hold A (5.50%), 1-year RCY", a.realized_compound_yield)
    table.percent("swap into B (5.75%), 1-year RCY", b.realized_compound_yield)
    table.value("pickup (bp)",
                1e4 * (b.realized_compound_yield - a.realized_compound_yield))

    table.heading("15.2.3 Riding an upward-sloping yield curve")
    spot = [0.040, 0.046, 0.050, 0.053]
    for n in range(1, len(spot) + 1):
        buy = (1 + spot[n - 1])**-n
        sell = 1.0 if n == 1 else (1 + spot[n - 2])**-(n - 1)
        print(f"  buy {n}-year zero, sell after 1 year on an unchanged curve: "
              f"{100 * (sell / buy - 1):5.2f}%")

    table.heading("15.3.2 Immunizing a 6-year, $5 million liability at 5.4%")
    y = 0.054
    liability = [cf.CashFlow(6.0, 5e6)]
    pv = cf.present_value_at_yield(liability, y)
    b1 = cf.coupon_bond(100, 0.04, 3)
    b2 = cf.coupon_bond(100, 0.06, 15)
    d1 = dur.macaulay_duration(b1, y)
    d2 = dur.macaulay_duration(b2, y)
    h = bp.immunize_two_bonds(pv, 6.0, d1, d2)
    units = [
        h.value1 / cf.present_value_at_yield(b1, y),
        h.value2 / cf.present_value_at_yield(b2, y)
    ]
    print(f"  bond durations {d1:.3f} and {d2:.3f}; hold ${h.value1:.0f} and "
          f"${h.value2:.0f}")
    assets = bp.aggregate([b1, b2], units)
    for dy in (-0.02, -0.01, 0.0, 0.01, 0.02):
        print(f"  {100 * (y + dy):7.2f}% "
              f"{bp.surplus(assets, liability, y + dy):16.2f}")

    table.heading("15.3.3 Cash matching a pension payout schedule")
    bonds = [
        cf.coupon_bond(1000, 0.035, 1),
        cf.coupon_bond(1000, 0.040, 2),
        cf.coupon_bond(1000, 0.045, 3),
        cf.coupon_bond(1000, 0.050, 4)
    ]
    prices = [998.0, 1001.5, 1003.0, 1008.0]
    payouts = [
        cf.CashFlow(float(t), a)
        for t, a in ((1, 2.0e6), (2, 2.2e6), (3, 2.4e6), (4, 2.6e6))
    ]
    units = bp.cash_match(bonds, payouts)
    for i, q in enumerate(units):
        print(f"  bond maturing year {i + 1}: {q:9.2f} bonds")
    table.value("cost of the dedicated portfolio ($)",
                sum(q * p for q, p in zip(units, prices)))

    table.heading("15.2.4 Contingent immunization: $100m, floor $125m in 5y")
    table.table(["rate %", "required $m", "cushion $m"], [[
        100 * r,
        bp.required_immunized_assets(125, r, 5),
        bp.contingent_cushion(100, 125, r, 5)
    ] for r in (0.05, 0.06, 0.07)],
                precision=2)

    table.heading("15.4 Interest rate swaps (semiannual, 3 years)")
    df = [(1 + (0.042 + 0.002 * k) / 2)**-k for k in range(1, 7)]
    par = bp.par_swap_rate(df, 0.5)
    table.percent("par swap rate", par)
    for fixed in (par - 0.005, par, par + 0.005):
        print(f"  pay {100 * fixed:5.3f}% fixed on $50m: value "
              f"{bp.pay_fixed_swap_value(5e7, fixed, df, 0.5):12.2f}")


if __name__ == "__main__":
    main()
