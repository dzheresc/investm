"""Worked examples for Chapter 1.

Run: python -m investm.ch01_introduction.examples
"""

import math

from investm.ch01_introduction import returns
from investm.core import stats
from investm.core import table


def main() -> None:
    table.heading("1.1.2 Return on a single holding")
    table.percent("HPR (40 -> 43, dividend 1.20)",
                  returns.holding_period_return(40.0, 43.0, 1.20))

    table.heading("1.1.2 Risk and return from a price history")
    r = returns.simple_returns([100, 104, 101, 108, 112, 109, 115])
    table.percent("mean periodic return", stats.mean(r))
    table.percent("standard deviation", stats.std_dev(r))
    table.percent("cumulative return", returns.cumulative_return(r))
    table.percent("geometric mean return", stats.geometric_mean_return(r))

    table.heading("1.1.2 Diversification: equal-weight portfolio of N stocks")
    print("  each stock: sigma = 40%, pairwise correlation = 0.25")
    rows = [(n, 100.0 * returns.equal_weight_portfolio_std_dev(n, 0.40, 0.25))
            for n in (1, 2, 5, 10, 20, 50, 100, 1000)]
    table.table(["N", "sigma_p %"], rows, precision=2)
    print(f"  floor as N grows: sqrt(rho) * sigma = "
          f"{100 * math.sqrt(0.25) * 0.40:.2f}%")


if __name__ == "__main__":
    main()
