"""Worked examples for Chapter 24.

Run: python -m investm.ch24_performance.examples
"""

import numpy as np

from investm.ch24_performance import performance as pf
from investm.core import table


def main() -> None:
    table.heading("24.2 Time-weighted vs dollar-weighted returns")
    value_end = [1060.0, 2010.0, 2150.0, 1890.0]
    flows = [1000.0, 0.0, -500.0, 0.0]
    r = pf.period_returns_from_values(1000.0, value_end, flows)
    for t, (x, f) in enumerate(zip(r, flows)):
        print(f"  quarter {t + 1}: {100 * x:+6.2f}%  flow {f:+7.0f}")
    table.percent("time-weighted (manager's skill)", pf.time_weighted_return(r))
    table.percent("dollar-weighted, per quarter (investor's timing)",
                  pf.dollar_weighted_return(1000.0, flows[:-1], value_end[-1]))

    table.heading("24.4 Risk-adjusted performance of three managers (60 mo)")
    rng = np.random.default_rng(3)
    n = 60
    m = rng.normal(0.007, 0.045, n)
    rf = np.full(n, 0.003)
    em = m - rf
    managers = {
        "A": rf + 0.002 + 0.7 * em + rng.normal(0, 0.015, n),
        "B": rf + 1.4 * em + rng.normal(0, 0.015, n),
        "C": rf - 0.001 + em + rng.normal(0, 0.03, n),
        "market": m,
    }
    rows = []
    for name, p in managers.items():
        s = pf.evaluate(list(p), list(m), list(rf))
        rows.append([
            name, 100 * s.mean_excess, s.beta, 100 * s.alpha, s.alpha_t_stat,
            s.sharpe, 100 * s.treynor, 100 * s.m_squared, s.appraisal_ratio
        ])
    table.table([
        "manager", "mean ex %", "beta", "alpha %", "t", "Sharpe", "Treynor %",
        "M^2 %", "apprais"
    ],
                rows,
                width=9,
                precision=3)

    table.heading("24.6 Market timing tests")
    timer = rf + np.where(em > 0, 1.3, 0.5) * em + rng.normal(0, 0.0075, n)
    tm = pf.treynor_mazuy(list(timer), list(m), list(rf))
    hm = pf.henriksson_merton(list(timer), list(m), list(rf))
    print(f"  Treynor-Mazuy:     beta {tm.beta:.3f}, timing {tm.timing:.3f} "
          f"(t {tm.timing_t_stat:.2f})")
    print(f"  Henriksson-Merton: beta {hm.beta:.3f}, timing {hm.timing:.3f} "
          f"(t {hm.timing_t_stat:.2f})")

    table.heading("24.7 Brinson attribution (stocks, bonds, cash)")
    a = pf.brinson_attribution([0.68, 0.25, 0.07], [0.60, 0.35, 0.05],
                               [0.052, 0.019, 0.011], [0.047, 0.021, 0.011])
    table.table(["segment", "allocation %", "selection %", "interact %"], [[
        s, 100 * x, 100 * y, 100 * z
    ] for s, x, y, z in zip(("stocks", "bonds",
                             "cash"), a.allocation, a.selection, a.interaction)
                                                                          ],
                width=13,
                precision=3)
    print(f"  portfolio {100 * a.portfolio_return:.3f}%, benchmark "
          f"{100 * a.benchmark_return:.3f}%, active {100 * a.total:.3f}%")

    table.heading("24.8 Returns-based style analysis")
    mk = rng.normal(0.007, 0.045, n)
    styles = np.column_stack([
        np.full(n, 0.003), 0.002 + 0.15 * mk + rng.normal(0, 0.008, n),
        0.9 * mk + rng.normal(0, 0.01, n), 1.2 * mk + rng.normal(0, 0.015, n)
    ])
    fund = styles @ np.array([0.10, 0.20, 0.25, 0.45]) + rng.normal(0, 0.003, n)
    fit = pf.style_analysis(fund, styles)
    for name, w in zip(("bills", "bonds", "value", "growth"), fit.weights):
        print(f"  {name:<7} {w:6.3f}")
    print(f"  R^2 {fit.r_squared:.4f} (true mix 0.10/0.20/0.25/0.45)")


if __name__ == "__main__":
    main()
