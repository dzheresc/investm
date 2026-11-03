"""Worked examples for Chapter 21.

Run: python -m investm.ch21_investment_companies.examples
"""

from investm.ch21_investment_companies import funds
from investm.core import table


def main() -> None:
    table.heading("21.2-21.3 NAV and closed-end fund discounts")
    nav = funds.net_asset_value(812.5e6, 12.5e6, 40e6)
    table.value("NAV per share ($812.5m assets, $12.5m debt, 40m sh)", nav)
    for price in (16.80, 19.10, 21.40):
        kind = "discount" if price < nav else "premium"
        print(f"  market price {price:.2f} -> "
              f"{100 * funds.premium_or_discount(price, nav):+.2f}% {kind}")

    table.heading("21.4 Open-end funds: loads and distributions")
    table.value("offering price, NAV 24.70, 5.75% load",
                funds.offering_price(24.70, 0.0575))
    table.percent("load as a share of the amount invested",
                  funds.load_as_fraction_of_investment(0.0575))
    table.percent("return: NAV 24.70 -> 25.90, $0.35 income, $0.80 gains",
                  funds.fund_return(24.70, 25.90, 0.35, 0.80))
    table.value("1,000 shares after reinvesting $1.15 at 25.90",
                funds.shares_after_reinvestment(1000, 1.15, 25.90))

    table.heading("21.5 What fees cost: $10,000 for N years at 7% gross")
    cdsc = [0.05, 0.04, 0.03, 0.02, 0.01]
    table.table(
        ["years", "index .05%", "no-load 1%", "A 5%+.5%", "B CDSC+1.5%"], [[
            n,
            funds.terminal_value(1e4, 0.07, 0.0005, n, 0),
            funds.terminal_value(1e4, 0.07, 0.01, n, 0),
            funds.terminal_value(1e4, 0.07, 0.005, n, 0.05),
            funds.terminal_value(1e4, 0.07, 0.015, n, 0, cdsc)
        ] for n in (1, 3, 5, 10, 20, 30)],
        precision=0)
    y = funds.breakeven_years(funds.FundTerms(0.005, 0.05),
                              funds.FundTerms(0.01, 0.0), 0.07)
    print(f"  A beats the 1% no-load after {y} years")


if __name__ == "__main__":
    main()
