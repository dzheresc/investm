"""Worked examples for Chapter 23.

Run: python -m investm.ch23_investment_management.examples
"""

import numpy as np

from investm.ch07_portfolio_analysis import efficient_set as es
from investm.ch23_investment_management import management as mg
from investm.core import regression
from investm.core import table


def main() -> None:
    table.heading("23.3 Risk tolerance (stocks 8.5% +/- 17%, bills 3.5%)")
    for w in (0.2, 0.4, 0.6, 0.8, 1.0):
        print(f"  holds {100 * w:3.0f}% stocks -> tau = "
              f"{mg.implied_risk_tolerance(w, 0.085, 0.17, 0.035):.3f}")

    table.heading("23.4 Strategic asset allocation (no short sales)")
    mu = np.array([0.035, 0.050, 0.075, 0.090])
    cov = regression.covariance_from_correlation(
        [0.01, 0.06, 0.16, 0.21], [[1, 0.1, 0, 0], [0.1, 1, 0.2, 0.1],
                                   [0, 0.2, 1, 0.7], [0, 0.1, 0.7, 1]])
    rows = []
    for tau in (0.05, 0.15, 0.30, 0.60):
        p = es.optimal_no_short_sales(mu, cov, tau)
        rows.append([tau] + list(p.weights) +
                    [100 * p.expected_return, 100 * p.std_dev])
    table.table(["tau", "bills", "bonds", "US eq", "intl eq", "E %", "sd %"],
                rows,
                width=8,
                precision=3)

    table.heading("23.4 Dynamic strategies over 2,000 simulated 10-year paths")
    rng = np.random.default_rng(23)
    rf = 0.003
    results = {
        "buy & hold 60/40": [],
        "constant mix 60/40": [],
        "CPPI m=3, floor 80%": []
    }
    for _ in range(2000):
        r = list(rng.normal(0.007, 0.045, 120))
        results["buy & hold 60/40"].append(
            mg.buy_and_hold(r, rf, 0.6).wealth[-1])
        results["constant mix 60/40"].append(
            mg.constant_mix(r, rf, 0.6, 0.05).wealth[-1])
        results["CPPI m=3, floor 80%"].append(
            mg.cppi(r, rf, 3.0, 0.8).wealth[-1])
    table.table(["strategy", "5th pct", "median", "95th pct", "min"],
                [[name] + [float(np.percentile(v, q))
                           for q in (5, 50, 95, 0)]
                 for name, v in results.items()],
                width=20,
                precision=3)


if __name__ == "__main__":
    main()
