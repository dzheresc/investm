"""Worked examples for Chapter 10.

Run: python -m investm.ch10_factor_models.examples
"""

import math

import numpy as np

from investm.ch10_factor_models import factor_models as fm
from investm.core import table

TRUTH = fm.FactorModel(
    np.array([0.004, 0.005, 0.003, 0.006, 0.004, 0.005]),
    np.array([[1.2, -0.4], [1.0, -0.3], [1.4, -0.6], [0.4, 0.5], [0.5, 0.3],
              [0.3, 0.6]]),
    np.array([0.0004, 0.0005, 0.0006, 0.0003, 0.0004, 0.0005]))


def main() -> None:
    table.heading("10.2-10.3 Moments implied by a two-factor model")
    fcov = np.diag([0.0009, 0.0004])
    w = [0.25, 0.25, 0.0, 0.25, 0.25, 0.0]
    f, r = fm.decompose_portfolio_variance(TRUTH, fcov, w)
    b = fm.portfolio_sensitivities(TRUTH, w)
    print(f"  portfolio sensitivities: GDP {b[0]:.3f}, INF {b[1]:.3f}")
    table.percent("factor share of portfolio variance", f / (f + r))

    rng = np.random.default_rng(2024)
    factors = np.column_stack(
        [rng.normal(0, 0.03, 240),
         rng.normal(0, 0.02, 240)])
    returns = (TRUTH.intercepts + factors @ TRUTH.sensitivities.T +
               np.sqrt(TRUTH.residual_variances) * rng.standard_normal(
                   (240, 6)))

    table.heading("10.4.1 Time-series estimation (240 months)")
    fit = fm.estimate_time_series(returns, factors)
    rows = [(i + 1, fit.sensitivities[i, 0], TRUTH.sensitivities[i, 0],
             fit.sensitivities[i, 1], TRUTH.sensitivities[i, 1],
             100 * math.sqrt(fit.residual_variances[i])) for i in range(6)]
    table.table(["stock", "b_GDP", "true", "b_INF", "true", "sd(e) %"],
                rows,
                precision=3)

    table.heading("10.4.2 Cross-sectional estimation with sector dummies")
    v = fm.estimate_cross_sectional(returns,
                                    fm.sector_dummies([0, 0, 0, 1, 1, 1], 2),
                                    False)
    for t in range(5):
        print(f"    month {t + 1}: cyclical {100 * v[t, 0]:+6.2f}  "
              f"defensive {100 * v[t, 1]:+6.2f}")

    table.heading("10.4.3 Factor-analytic (principal components)")
    pf = fm.estimate_principal_components(returns, 2)
    print("  eigenvalues: " + " ".join(f"{x:.5f}" for x in pf.eigenvalues))
    table.percent("variance explained by 2 statistical factors",
                  pf.explained_fraction)


if __name__ == "__main__":
    main()
