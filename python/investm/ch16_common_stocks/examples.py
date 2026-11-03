"""Worked examples for Chapter 16.

Run: python -m investm.ch16_common_stocks.examples
"""

from investm.ch16_common_stocks import betas
from investm.ch16_common_stocks import corporate as corp
from investm.core import table


def main() -> None:
    table.heading("16.1.3 Cumulative vs majority voting (2.4m shares, 7 seats)")
    shares = 2_400_000
    for seats in range(1, 5):
        need = corp.cumulative_voting_shares_needed(shares, 7, seats)
        print(f"  seats {seats}: cumulative needs {need:9d} shares "
              f"({100 * need / shares:5.2f}%)")
    print(f"  majority voting: any seat needs "
          f"{corp.majority_voting_shares_needed(shares)} shares")

    table.heading("16.3 Splits and stock dividends")
    table.value("price after 3-for-2 split of $87",
                corp.price_after_split(87, 1.5))
    table.value(
        "price after 15% stock dividend on $46",
        corp.price_after_split(46, corp.split_ratio_from_stock_dividend(0.15)))
    prices = [120.0, 123.0, 41.5, 42.2, 21.4, 21.9]
    ratios = [1.0, 1.0, 3.0, 1.0, 2.0, 1.0]
    adj = corp.split_adjust_prices(prices, ratios)
    for t, (p, k, a) in enumerate(zip(prices, ratios, adj)):
        ret = f"{100 * (a / adj[t - 1] - 1):8.2f}%" if t else ""
        print(f"  {t:4d} {p:9.2f} {k:6.1f} {a:9.3f} {ret}")

    table.heading("16.4 A rights offering: 5 rights + $32 buy one new share")
    terms = corp.RightsTerms(5, 32.0)
    ex = corp.ex_rights_price(41.0, terms)
    table.value("rights-on price", 41.0)
    table.value("theoretical ex-rights price", ex)
    table.value("value of a right (rights-on formula)",
                corp.right_value_cum(41.0, terms))
    table.value("value of a right (ex-rights formula)",
                corp.right_value_ex(ex, terms))

    table.heading("16.7 Estimating and adjusting betas")
    market = [
        0.031, -0.012, 0.024, 0.008, -0.035, 0.042, 0.015, -0.021, 0.019, 0.027,
        -0.008, 0.011
    ]
    stock = [
        0.048, -0.025, 0.029, 0.021, -0.061, 0.066, 0.012, -0.027, 0.034, 0.031,
        -0.019, 0.024
    ]
    fit = betas.estimate_beta(stock, market)
    print(f"  raw beta {fit.beta:.3f} (se {fit.beta_std_error:.3f}), "
          f"alpha {fit.alpha:.4f}, R^2 {fit.r_squared:.3f}")
    table.value("adjusted beta (2/3 raw + 1/3 x 1.0)",
                betas.shrink_beta(fit.beta))
    table.value("Vasicek beta (prior 1.0, sd 0.35)",
                betas.vasicek_beta(fit.beta, fit.beta_std_error, 1.0, 0.35))
    p = betas.beta_persistence([0.45, 0.70, 0.95, 1.10, 1.35, 1.60, 1.90],
                               [0.62, 0.81, 0.97, 1.06, 1.22, 1.39, 1.57])
    print(f"  later = {p.alpha:.3f} + {p.beta:.3f} x earlier: betas regress "
          f"toward 1")

    table.heading("16.7 Leverage and beta (25% tax)")
    asset = betas.unlevered_beta(1.30, 0.60, 0.25)
    table.value("asset beta of a 1.30 equity beta at D/E 0.60", asset)
    for de in (0.0, 0.3, 0.6, 1.0, 1.5):
        print(f"  D/E {de:.1f} -> equity beta "
              f"{betas.levered_beta(asset, de, 0.25):.3f}")


if __name__ == "__main__":
    main()
