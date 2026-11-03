"""Worked examples for Chapter 6.

Run: python -m investm.ch06_portfolio_selection.examples
"""

import numpy as np

from investm.ch06_portfolio_selection import portfolio
from investm.ch06_portfolio_selection import risk_measures as rm
from investm.ch06_portfolio_selection import utility
from investm.ch06_portfolio_selection.utility import Utility
from investm.ch06_portfolio_selection.utility import UtilityKind
from investm.core import regression
from investm.core import table


def main() -> None:
    table.heading("6.1 Initial and terminal wealth")
    shares, prices, end = [150, 300, 80], [30, 22, 75], [33, 21, 84]
    w = portfolio.weights_from_holdings(shares, prices)
    print("  weights: " + " ".join(f"{x:.4f}" for x in w))
    table.percent("portfolio return (W1 - W0) / W0",
                  portfolio.portfolio_return_from_prices(shares, prices, end))

    table.heading("6.3 Utility, certainty equivalents and risk premiums")
    wealth, probs = [70000, 130000], [0.5, 0.5]
    rows = []
    for name, u in (("risk neutral, U = W", Utility(UtilityKind.LINEAR)),
                    ("log utility", Utility(UtilityKind.LOG)),
                    ("power, g = 4", Utility(UtilityKind.POWER, 4.0)),
                    ("exponential, a = 1e-4",
                     Utility(UtilityKind.EXPONENTIAL,
                             1e-4)), ("risk seeking, U = W^2",
                                      Utility(UtilityKind.CONVEX))):
        rows.append((name, int(utility.certainty_equivalent(u, wealth, probs)),
                     int(utility.risk_premium(u, wealth, probs))))
    table.table(["investor", "CE $", "premium $"], rows, width=22)

    table.heading("6.4 Indifference curves (U = E - sigma^2 / tau)")
    sig = [0.0, 0.05, 0.10, 0.15, 0.20, 0.25]
    curves = [utility.indifference_curve(0.06, sig, t) for t in (0.25, 0.5, 1)]
    rows = [(100 * s, *(100 * c[i] for c in curves)) for i, s in enumerate(sig)]
    table.table(["sigma %", "tau=0.25", "tau=0.50", "tau=1.00"],
                rows,
                precision=2)

    table.heading("6.5 Expected return and standard deviation of a portfolio")
    mu = [0.08, 0.12, 0.15]
    sd = [0.15, 0.22, 0.30]
    cov = regression.covariance_from_correlation(
        sd, [[1, 0.3, 0.2], [0.3, 1, 0.4], [0.2, 0.4, 1]])
    w = [0.5, 0.3, 0.2]
    table.percent("expected return", portfolio.portfolio_expected_return(w, mu))
    table.percent("standard deviation", portfolio.portfolio_std_dev(w, cov))
    for rho in (1.0, 0.5, 0.0, -0.5, -1.0):
        print(f"    rho = {rho:+.1f}  sigma_P = "
              f"{100 * portfolio.two_asset_std_dev(0.6, 0.2, 0.3, rho):6.3f}%")

    table.heading("Institutional issues: alternative risk measures")
    rng = np.random.default_rng(11)
    r = 0.008 + 0.03 * rng.standard_t(4.0, 5000)
    m, s = float(r.mean()), float(r.std(ddof=1))
    table.percent("standard deviation", s)
    table.percent("downside deviation below 0", rm.downside_deviation(r, 0))
    table.percent("99% VaR, historical", rm.historical_var(r, 0.99))
    table.percent("99% VaR, normal approximation", rm.normal_var(m, s, 0.99))
    table.percent("99% expected shortfall, historical",
                  rm.historical_expected_shortfall(r, 0.99))
    table.percent("99% expected shortfall, normal",
                  rm.normal_expected_shortfall(m, s, 0.99))


if __name__ == "__main__":
    main()
