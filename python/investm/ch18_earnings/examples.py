"""Worked examples for Chapter 18.

Run: python -m investm.ch18_earnings.examples
"""

import numpy as np

from investm.ch18_earnings import earnings as ea
from investm.core import table


def main() -> None:
    table.heading("18.3 Dividends follow earnings slowly (Lintner)")
    eps = [3.10, 3.45, 2.80, 3.90, 4.25, 3.70, 4.60, 5.05, 4.40, 5.30]
    dps = [1.20, 1.27, 1.25, 1.36, 1.49, 1.53, 1.65, 1.82, 1.85, 1.98]
    m = ea.fit_lintner(eps, dps)
    print(f"  dD = {m.a:.4f} + {m.adjustment:.3f} ({m.target_payout:.3f} E - "
          f"D_prev),  R^2 {m.r_squared:.3f}")
    for t in range(1, len(eps)):
        print(f"  {t + 1:4d} {eps[t]:7.2f} {dps[t]:7.2f} "
              f"{m.next_dividend(dps[t - 1], eps[t]):10.3f}")

    table.heading("18.4 Naive earnings forecasts")
    table.value("random walk with drift, next year EPS",
                ea.random_walk_forecast([2.10, 2.35, 2.28, 2.61, 2.90]))
    q = [0.42, 0.61, 0.55, 0.78, 0.47, 0.66, 0.58, 0.85, 0.50, 0.71, 0.63, 0.90]
    table.value("seasonal random walk, next quarter EPS",
                ea.seasonal_random_walk_forecast(q))
    for actual in (0.49, 0.55, 0.60):
        print(f"  actual {actual:.2f} -> SUE "
              f"{ea.standardized_unexpected_earnings(q, actual):+.2f}")

    table.heading("18.5 Returns after earnings announcements, by SUE quintile")
    rng = np.random.default_rng(18)
    sue = rng.normal(0.0, 1.5, 2000)
    car = 0.005 * sue + rng.normal(0.0, 0.03, 2000)
    group = ea.sort_into_groups(list(sue), 5)
    mean_sue = ea.group_means(list(sue), group, 5)
    mean_car = ea.group_means(list(car), group, 5)
    for k in range(5):
        print(f"  {k + 1:9d} {mean_sue[k]:10.2f} {100 * mean_car[k]:15.2f}%")


if __name__ == "__main__":
    main()
