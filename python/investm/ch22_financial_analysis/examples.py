"""Worked examples for Chapter 22.

Run: python -m investm.ch22_financial_analysis.examples
"""

import numpy as np

from investm.ch22_financial_analysis import ratios
from investm.ch22_financial_analysis import technical as ta
from investm.core import stats
from investm.core import table


def _firm(debt: float, interest: float) -> ratios.Financials:
    return ratios.Financials(revenue=2400,
                             ebit=312,
                             interest_expense=interest,
                             income_tax=0.25 * (312 - interest),
                             total_assets=1950,
                             equity=1950 - debt - 150,
                             current_assets=640,
                             inventory=210,
                             current_liabilities=380,
                             total_debt=debt,
                             dividends=70,
                             shares=120,
                             price=18.40)


def main() -> None:
    table.heading("22.3 Ratio analysis and the DuPont decomposition")
    lo, hi = _firm(300, 18), _firm(1000, 64)
    a, b = ratios.compute_ratios(lo), ratios.compute_ratios(hi)
    da, db = ratios.decompose_roe(lo), ratios.decompose_roe(hi)
    rows = [
        ("tax burden", da.tax_burden, db.tax_burden),
        ("interest burden", da.interest_burden, db.interest_burden),
        ("operating margin", da.operating_margin, db.operating_margin),
        ("asset turnover", da.asset_turnover, db.asset_turnover),
        ("equity multiplier", da.equity_multiplier, db.equity_multiplier),
        ("ROE", a.roe, b.roe),
        ("ROA", a.roa, b.roa),
        ("debt / equity", a.debt_to_equity, b.debt_to_equity),
        ("interest coverage", a.interest_coverage, b.interest_coverage),
        ("current ratio", a.current_ratio, b.current_ratio),
        ("EPS", a.eps, b.eps),
        ("P/E at $18.40", a.price_earnings, b.price_earnings),
        ("price / book", a.price_to_book, b.price_to_book),
    ]
    for name, x, y in rows:
        print(f"  {name:<22} {x:12.4f} {y:12.4f}")

    table.heading("22.4 Technical analysis on a simulated random walk")
    rng = np.random.default_rng(22)
    p = list(100 * np.exp(
        np.concatenate([[0], np.cumsum(rng.normal(0.0003, 0.012, 1000))])))
    hold = [b / a - 1 for a, b in zip(p, p[1:])]
    print(f"  {'buy and hold':<18} {100 * stats.mean(hold):11.4f}% "
          f"{100 * stats.std_dev(hold):11.4f}%")
    for n in (20, 50, 200):
        r = ta.moving_average_rule_returns(p, n, 0.0001)
        print(f"  {n:3d}-day MA rule     {100 * stats.mean(r):11.4f}% "
              f"{100 * stats.std_dev(r):11.4f}%")
    print("  (prices follow a random walk here, so any edge is luck;\n"
          "   try other seeds)")


if __name__ == "__main__":
    main()
