"""Worked examples for Chapter 7.

Run: python -m investm.ch07_portfolio_analysis.examples
"""

import math

import numpy as np

from investm.ch07_portfolio_analysis import efficient_set as es
from investm.ch07_portfolio_analysis import market_model as mm
from investm.core import regression
from investm.core import table

MU = np.array([0.045, 0.065, 0.085, 0.095, 0.110])
SD = [0.06, 0.10, 0.16, 0.20, 0.26]
CORR = [[1.00, 0.40, 0.10, 0.05, 0.00], [0.40, 1.00, 0.35, 0.30, 0.20],
        [0.10, 0.35, 1.00, 0.75, 0.60], [0.05, 0.30, 0.75, 1.00, 0.65],
        [0.00, 0.20, 0.60, 0.65, 1.00]]
COV = regression.covariance_from_correlation(SD, CORR)


def _show(label: str, p: es.PortfolioPoint) -> None:
    w = " ".join(f"{x:+6.3f}" for x in p.weights)
    print(f"  {label:<26} E={100 * p.expected_return:6.2f}% "
          f"sd={100 * p.std_dev:6.2f}%  w: {w}")


def main() -> None:
    table.heading("7.1 / Appendix A Efficient set, short sales allowed")
    f = es.UnconstrainedFrontier.create(MU, COV)
    _show("minimum variance", f.minimum_variance())
    for e in (0.07, 0.09, 0.11, 0.13):
        _show(f"target E = {100 * e:.0f}%", f.for_target_return(e))

    table.heading("7.1 / Appendix A Efficient set, no short sales")
    for tau in (0.0, 0.05, 0.1, 0.2, 0.4, 0.8, 1.6, 5.0):
        _show(f"tau = {tau:.2f}", es.optimal_no_short_sales(MU, COV, tau))

    table.heading("7.3 The market model on simulated monthly data")
    rng = np.random.default_rng(3)
    index = rng.normal(0.008, 0.045, 120)
    models = []
    rows = []
    for b in (0.6, 0.9, 1.1, 1.4):
        stock = 0.001 + b * index + rng.normal(0, 0.06, 120)
        m = mm.fit_market_model(stock, index)
        models.append(m)
        rows.append((b, m.beta, 100 * m.alpha, m.r_squared,
                     100 * math.sqrt(m.residual_variance)))
    table.table(["true beta", "beta", "alpha %", "R^2", "sd(e) %"],
                rows,
                precision=3)

    table.heading("7.4 Portfolio market and unique risk")
    r = mm.portfolio_risk([0.25] * 4, models, 0.045**2)
    print(f"  equal-weight portfolio: beta {r.beta:.3f}")
    table.percent("market risk (sd)", math.sqrt(r.market_variance))
    table.percent("unique risk (sd)", math.sqrt(r.unique_variance))
    table.percent("total risk (sd)", math.sqrt(r.total_variance))

    table.heading("7.4.5 Random diversification")
    for n in (1, 2, 5, 10, 20, 50, 100, 500):
        v = mm.random_diversification_variance(n, 0.16, 0.03)
        print(f"  N = {n:3d}  sigma_P = {100 * math.sqrt(v):6.2f}%")


if __name__ == "__main__":
    main()
