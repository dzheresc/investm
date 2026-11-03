"""Worked examples for Chapter 14.

Run: python -m investm.ch14_bond_analysis.examples
"""

import numpy as np

from investm.ch05_riskless_securities import cash_flows as cf
from investm.ch14_bond_analysis import credit
from investm.ch14_bond_analysis import duration as dur
from investm.core import regression
from investm.core import table


def main() -> None:
    table.heading("14.2 Bond pricing theorems (yield starts at 5.5%)")
    rows = []
    for name, coupon, years in (("3% 5-year", 0.03, 5), ("3% 15-year", 0.03,
                                                         15), ("3% 30-year",
                                                               0.03, 30),
                                ("9% 15-year", 0.09, 15), ("zero 15-year", 0.0,
                                                           15)):
        f = cf.coupon_bond(100, coupon, years, 2)
        p0 = cf.present_value_at_yield(f, 0.055, 2)
        rows.append([name] + [
            100 * (cf.present_value_at_yield(f, 0.055 + dy, 2) / p0 - 1)
            for dy in (-0.01, -0.005, 0.005, 0.01)
        ])
    table.table(["bond", "-100bp", "-50bp", "+50bp", "+100bp"],
                rows,
                precision=2)

    table.heading("14.3 Duration and convexity of a 4.5% 12-year bond at 5.8%")
    f = cf.coupon_bond(100, 0.045, 12, 2)
    y = 0.058
    p = cf.present_value_at_yield(f, y, 2)
    d = dur.modified_duration(f, y, 2)
    c = dur.convexity(f, y, 2)
    table.value("price", p)
    table.value("Macaulay duration (years)", dur.macaulay_duration(f, y, 2))
    table.value("modified duration", d)
    table.value("convexity", c)
    table.value("PVBP (per 100 face)", dur.price_value_of_basis_point(f, y, 2))
    table.table(["dy %", "actual", "duration", "dur+convex"], [[
        100 * dy,
        cf.present_value_at_yield(f, y + dy, 2) - p,
        dur.approx_price_change(p, d, 0, dy),
        dur.approx_price_change(p, d, c, dy)
    ] for dy in (-0.03, -0.01, 0.01, 0.03)])

    table.heading("14.4 Promised vs expected yield (6% 7-year, priced at 91)")
    rows = []
    for prob in (0.0, 0.01, 0.02, 0.04):
        dd = credit.decompose_yield(100, 0.06, 7, 91, prob, 0.4, 0.042)
        rows.append([
            100 * prob, 100 * dd.promised_yield, 100 * dd.expected_yield,
            100 * dd.default_premium, 100 * dd.risk_premium
        ])
    table.table(["P(def) %", "promised", "expected", "def.prem", "risk prem"],
                rows,
                precision=3)
    table.percent("implied 1-yr default prob, 7.5% vs 4.2%, 40% recovery",
                  credit.implied_default_probability(0.075, 0.042, 0.4))

    table.heading("14.5 Yield spread regression on bond attributes")
    x = np.array([[3, 0.30, 6.2], [10, 0.45, 5.8], [7, 0.60,
                                                    5.1], [5, 0.25, 6.8],
                  [12, 0.55, 5.5], [2, 0.70, 4.9], [8, 0.35, 6.0],
                  [15, 0.40, 6.4], [4, 0.65, 5.0], [9, 0.50, 5.6]])
    spread = np.array([
        0.0072, 0.0151, 0.0208, 0.0058, 0.0196, 0.0225, 0.0105, 0.0149, 0.0221,
        0.0163
    ])
    fit = regression.multiple_ols(x, spread)
    for name, b, se in zip(("intercept", "maturity", "debt/assets", "log size"),
                           fit.coefficients, fit.std_errors):
        print(f"  {name:<12} {b:+9.5f} (se {se:.5f})")
    table.value("R^2", fit.r_squared)

    table.heading("14.6 A discriminant credit score from two ratios")
    sound = [[7.5, 0.32], [5.9, 0.41], [10.2, 0.22], [6.6, 0.38], [8.8, 0.29]]
    distressed = [[1.4, 0.71], [2.3, 0.62], [0.9, 0.83], [1.8, 0.66],
                  [2.6, 0.58]]
    lda = credit.LinearDiscriminant.fit(sound, distressed)
    print(f"  score = {lda.weights[0]:.4f} x coverage {lda.weights[1]:+.4f} "
          f"x debt/assets, cutoff {lda.cutoff:.4f}")
    for firm in ([6.0, 0.35], [4.0, 0.45], [1.5, 0.40]):
        kind = "sound" if lda.is_group_a(firm) else "distressed"
        print(f"  coverage {firm[0]:.1f}, debt/assets {firm[1]:.2f} -> score "
              f"{lda.score(firm):7.3f} : {kind}")


if __name__ == "__main__":
    main()
