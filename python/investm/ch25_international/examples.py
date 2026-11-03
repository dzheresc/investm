"""Worked examples for Chapter 25.

Run: python -m investm.ch25_international.examples
"""

import math

from investm.ch25_international import international as intl
from investm.core import table


def main() -> None:
    table.heading("25.3 Returns in domestic currency")
    print("  foreign stock returns 9% in local currency")
    for s0, s1 in ((1.08, 1.17), (1.08, 1.08), (1.08, 0.97)):
        e = intl.currency_return(s0, s1)
        print(f"  spot {s0:.2f} -> {s1:.2f} ({100 * e:+.2f}%): domestic return "
              f"{100 * intl.domestic_return(0.09, e):+.2f}%")
    for rho in (-0.3, 0.0, 0.3):
        sd = math.sqrt(intl.domestic_return_variance(0.18, 0.10, rho))
        print(f"  sd local 18%, sd currency 10%, rho {rho:+.1f} -> sd "
              f"{100 * sd:.2f}%")

    table.heading("25.4 Interest rate parity and hedging")
    s = 1.08
    f = intl.parity_forward(s, 0.045, 0.025, 1)
    table.value("1-year forward (4.5% home, 2.5% abroad)", f)
    table.percent("forward premium", intl.forward_premium(s, f))
    print("  foreign stock: expected 8%, realized 11%")
    for s1 in (0.97, 1.08, 1.17):
        unhedged = intl.domestic_return(0.11, intl.currency_return(s, s1))
        hedged = intl.hedged_return(0.11, 0.08, s, f, s1)
        print(f"  spot ends {s1:.2f}: unhedged {100 * unhedged:+6.2f}%, "
              f"hedged {100 * hedged:+6.2f}%")

    table.heading("25.5 Purchasing power parity")
    table.percent("expected change, 4% home vs 1.5% foreign inflation",
                  intl.ppp_expected_change(0.04, 0.015))
    print(f"  real exchange rate: {intl.real_exchange_rate(1.08, 100, 100):.4f}"
          f" -> {intl.real_exchange_rate(1.02, 101.5, 104):.4f}")

    table.heading("25.6 Diversifying abroad (home sd 15%, foreign sd 19%)")
    table.table(["foreign %", "rho 0.2", "rho 0.5", "rho 0.8"],
                [[100 * w] + [
                    100 * intl.two_market_std_dev(w, 0.15, 0.19, rho)
                    for rho in (0.2, 0.5, 0.8)
                ]
                 for w in (0.0, 0.2, 0.4, 0.6, 0.8, 1.0)],
                width=10,
                precision=2)
    for rho in (0.2, 0.5, 0.8):
        w = intl.minimum_risk_foreign_weight(0.15, 0.19, rho)
        print(f"  rho {rho:.1f}: minimum risk at {100 * w:.1f}% abroad (sd "
              f"{100 * intl.two_market_std_dev(w, 0.15, 0.19, rho):.2f}%)")


if __name__ == "__main__":
    main()
