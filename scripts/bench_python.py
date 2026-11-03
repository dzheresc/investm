"""Times the Python mirrors of the hot paths benchmarked in cpp/bench.

Usage: python3 scripts/bench_python.py [path/to/python]
The optional argument is the directory holding the `investm` package, so
two checkouts can be compared.
"""

import os
import sys
import timeit

import numpy as np

sys.path.insert(
    0, sys.argv[1] if len(sys.argv) > 1 else os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "..", "python"))

# pylint: disable=wrong-import-position
from investm.ch07_portfolio_analysis import efficient_set as es
from investm.ch08_riskfree_borrowing_lending import riskfree as rfm
from investm.ch19_options import pricing as pr
from investm.ch24_performance import performance as perf


def _problem(rng, n):
    b = rng.normal(size=(n + 20, n))
    return rng.uniform(0.04, 0.16, n), b.T @ b / (n + 20) * 0.04


def main() -> None:
    rng = np.random.default_rng(1)
    cases = {}
    for n in (20, 100):
        mu, cov = _problem(rng, n)
        taus = np.geomspace(0.01, 10, 8)
        cases[f"ch07/optimal_no_short/{n}"] = (
            lambda mu=mu, cov=cov: es.optimal_no_short_sales(mu, cov, 0.3))
        cases[f"ch07/efficient_set_no_short/{n}x8"] = (
            lambda mu=mu, cov=cov, taus=taus: es.efficient_set_no_short_sales(
                mu, cov, taus))
        cases[f"ch08/tangency_no_short/{n}"] = (
            lambda mu=mu, cov=cov: rfm.tangency_portfolio_no_short_sales(
                mu, cov, 0.03))
    spec = pr.OptionSpec(pr.OptionType.PUT, 100.0, 100.0, 0.05, 0.2, 1.0)
    cases["ch19/binomial_american/1000"] = lambda: pr.binomial_price(
        spec, 1000, True)
    x = rng.normal(0.01, 0.04, size=(120, 8))
    f = x @ np.full(8, 1 / 8) + rng.normal(0, 0.005, 120)
    cases["ch24/style_analysis/120x8"] = lambda: perf.style_analysis(f, x)
    print(f"{'case':36s} {'best of 5':>12s}")
    for name, fn in cases.items():
        best = min(timeit.repeat(fn, number=1, repeat=5))
        print(f"{name:36s} {best * 1e3:9.2f} ms")


if __name__ == "__main__":
    main()
