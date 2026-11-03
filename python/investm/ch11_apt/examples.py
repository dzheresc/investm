"""Worked examples for Chapter 11.

Run: python -m investm.ch11_apt.examples
"""

import numpy as np

from investm.ch11_apt import apt
from investm.core import table


def main() -> None:
    table.heading("11.1 An arbitrage opportunity in a one-factor world")
    b = np.array([[0.9], [1.1], [1.5], [0.5]])
    mu = np.array([0.10, 0.12, 0.20, 0.07])
    x = apt.find_arbitrage_portfolio(mu, b, 100000.0)
    for i, v in enumerate(x):
        print(f"    stock {i + 1}: b={b[i, 0]:.2f} E={100 * mu[i]:5.2f}% "
              f"trade {v:+12.2f}")
    print(f"  net cost {abs(x.sum()):.2f}, factor exposure "
          f"{float(b[:, 0] @ x):.2e}, expected profit ${float(mu @ x):.2f}")

    table.heading("11.4 A two-factor APT plane")
    b2 = np.array([[1.0, 0.2], [0.5, 1.0], [1.2, -0.3], [0.8, 0.7], [0.3, 0.1],
                   [1.5, 0.4]])
    mu2 = np.array(
        [apt.apt_expected_return(0.035, [0.045, 0.015], row) for row in b2])
    fit = apt.fit_apt_pricing(mu2, b2)
    table.percent("fitted lambda_0", fit.lambda0)
    table.percent("fitted lambda_1", fit.lambdas[0])
    table.percent("fitted lambda_2", fit.lambdas[1])
    for k in range(2):
        w = apt.pure_factor_portfolio(b2, k)
        print(f"  pure factor-{k + 1} portfolio earns {100 * w @ mu2:.3f}%")


if __name__ == "__main__":
    main()
