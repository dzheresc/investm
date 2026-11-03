"""Worked examples for Chapter 9.

Run: python -m investm.ch09_capm.examples
"""

import math

import numpy as np

from investm.ch07_portfolio_analysis import efficient_set as es
from investm.ch08_riskfree_borrowing_lending import riskfree as rfm
from investm.ch09_capm import capm
from investm.core import regression
from investm.core import table

NAMES = ["Utility", "Bank", "Retail", "Tech", "Biotech"]
SD = np.array([0.14, 0.20, 0.24, 0.32, 0.40])
CORR = [[1.00, 0.45, 0.40, 0.30, 0.20], [0.45, 1.00, 0.55, 0.45, 0.30],
        [0.40, 0.55, 1.00, 0.50, 0.35], [0.30, 0.45, 0.50, 1.00, 0.55],
        [0.20, 0.30, 0.35, 0.55, 1.00]]
COV = regression.covariance_from_correlation(SD, CORR)
MARKET = np.array([0.15, 0.25, 0.20, 0.30, 0.10])


def main() -> None:
    table.heading("9.2-9.3 Equilibrium: CML, SML and implied returns")
    rf, premium = 0.03, 0.055
    mu = capm.implied_expected_returns(COV, MARKET, rf, premium)
    b = capm.betas(COV, MARKET)
    sd_m = math.sqrt(MARKET @ COV @ MARKET)
    print(f"  rf = 3%, E_M - rf = 5.5%, sigma_M = {100 * sd_m:.2f}%")
    rows = []
    for i, name in enumerate(NAMES):
        mkt, own = capm.partition_risk(SD[i]**2, b[i], sd_m**2)
        rows.append((name, b[i], 100 * mu[i], 100 * SD[i], 100 * mkt / SD[i]**2,
                     100 * own / SD[i]**2))
    table.table(["stock", "beta", "E %", "sd %", "mkt var %", "own var %"],
                rows,
                precision=3)
    t = rfm.tangency_portfolio(mu, COV, rf)
    print("  tangency weights: " + " ".join(f"{w:.3f}" for w in t.weights) +
          "  (= market weights)")

    table.heading("9.3 Alpha: a stock off the SML")
    table.percent("SML required return",
                  capm.security_market_line(0.03, 0.085, 1.3))
    table.percent("alpha of an 11.5% forecast",
                  capm.alpha(0.115, 0.03, 0.085, 1.3))

    table.heading("Appendix A.1 The zero-beta CAPM")
    f = es.UnconstrainedFrontier.create(mu, COV)
    ez = capm.zero_beta_return(f.a, f.b, f.c, 0.10)
    table.percent("zero-beta portfolio expected return", ez)
    for beta in (0.0, 0.5, 1.0, 1.5):
        print(
            f"  beta {beta:.1f} -> E = "
            f"{100 * capm.zero_beta_security_market_line(ez, 0.10, beta):.3f}%")


if __name__ == "__main__":
    main()
