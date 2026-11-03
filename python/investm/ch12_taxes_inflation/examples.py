"""Worked examples for Chapter 12.

Run: python -m investm.ch12_taxes_inflation.examples
"""

from investm.ch12_taxes_inflation import inflation as inf
from investm.ch12_taxes_inflation import taxes
from investm.core import table


def main() -> None:
    table.heading("12.1 A progressive schedule: marginal vs average rates")
    s = taxes.TaxSchedule([
        taxes.TaxBracket(0, 0.10),
        taxes.TaxBracket(25000, 0.18),
        taxes.TaxBracket(70000, 0.27),
        taxes.TaxBracket(160000, 0.36)
    ])
    table.table(
        ["income", "tax", "marginal %", "average %"],
        [[y, s.tax(y), 100 * s.marginal_rate(y), 100 * s.average_rate(y)]
         for y in (15000.0, 45000.0, 90000.0, 250000.0)],
        precision=2)

    table.heading("12.1.3 Tax-exempt bonds: taxable-equivalent yield")
    for fed in (0.12, 0.24, 0.32, 0.37):
        t = taxes.combined_tax_rate(fed, 0.06, True)
        print(f"  federal {100 * fed:4.0f}% + state 6% -> combined "
              f"{100 * t:5.2f}%, taxable-equivalent of 3.20% muni = "
              f"{100 * taxes.taxable_equivalent_yield(0.032, t):5.2f}%")
    table.percent("break-even rate, 5.00% taxable vs 3.20% muni",
                  taxes.breakeven_tax_rate(0.05, 0.032))

    table.heading("12.1.2 The value of deferring tax (8% pretax, 30% tax)")
    table.table(["years", "annual tax", "tax at end", "no tax", "eff. %"], [[
        n,
        taxes.wealth_taxed_annually(1000, 0.08, 0.3, n),
        taxes.wealth_taxed_at_end(1000, 0.08, 0.3, n),
        taxes.wealth_untaxed(1000, 0.08, n),
        100 * taxes.effective_deferred_rate(0.08, 0.3, n)
    ] for n in (1.0, 5.0, 10.0, 20.0, 40.0)],
                precision=3)

    table.heading("12.2 Inflation and real returns")
    index = [180.0, 185.4, 192.8, 194.7, 201.5, 213.6]
    nominal = [0.12, -0.04, 0.09, 0.15, 0.03]
    infl = inf.inflation_rates(index)
    real = inf.real_returns(nominal, infl)
    table.table(["year", "nominal %", "inflation %", "real %"],
                [[t + 1, 100 * n, 100 * i, 100 * r]
                 for t, (n, i, r) in enumerate(zip(nominal, infl, real))],
                precision=2)
    table.percent("cumulative inflation", inf.cumulative_inflation(index))
    table.value("purchasing power of $1", inf.purchasing_power(index))
    fit = inf.inflation_beta(nominal, infl)
    print(f"  inflation beta of this asset: {fit.beta:.3f} "
          f"(R^2 {fit.r_squared:.3f})")

    table.heading("12.2.4 Taxes on nominal returns (30% tax)")
    for r, i in ((0.05, 0.01), (0.07, 0.03), (0.10, 0.06)):
        print(
            f"  nominal {100 * r:4.1f}%, inflation {100 * i:4.1f}%: "
            f"real after tax {100 * inf.real_after_tax_return(r, 0.3, i):5.2f}"
            f"%, effective tax on real return "
            f"{100 * inf.effective_tax_on_real_return(r, 0.3, i):5.1f}%")

    table.heading("12.2.5 An inflation-indexed bond")
    for f in inf.indexed_bond_cash_flows(1000, 0.015, 100,
                                         [102.5, 105.1, 107.2, 110.4]):
        print(f"  t={f.time:.0f}  pays {f.amount:9.2f}")
    table.percent("break-even inflation, 4.5% nominal vs 1.5% real",
                  inf.breakeven_inflation(0.045, 0.015))


if __name__ == "__main__":
    main()
