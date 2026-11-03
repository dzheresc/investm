"""Worked examples for Chapter 20.

Run: python -m investm.ch20_futures.examples
"""

from investm.ch19_options.payoffs import OptionType
from investm.ch20_futures import futures as fu
from investm.core import table


def main() -> None:
    table.heading("20.2 Marking to market: short 3 contracts x 1,000 at 72.40")
    print("  initial margin $9,000, maintenance $6,500")
    days = fu.mark_to_market(72.40, [72.10, 73.05, 74.20, 73.60, 75.10, 74.00],
                             -3, 1000, 9000, 6500)
    table.table(["day", "settle", "gain", "margin call", "equity"],
                [[t + 1, d.settlement_price, d.gain, d.margin_call, d.equity]
                 for t, d in enumerate(days)],
                precision=2)

    table.heading("20.5 Futures prices and the cost of carry")
    table.value("gold: S 1,950, r 4.5%, storage 0.3%, 9 months",
                fu.cost_of_carry_price(1950, 0.045, 0, 0.003, 0.75))
    table.value("index: S 4,800, r 4.5%, yield 1.6%, 6 months",
                fu.cost_of_carry_price(4800, 0.045, 0.016, 0, 0.5))
    divs = [fu.Dividend(1 / 12, 6.0), fu.Dividend(4 / 12, 8.0)]
    table.value("index with two discrete dividends, 5 months",
                fu.index_futures_price(950, 0.045, divs, 5 / 12))
    band = fu.no_arbitrage_band(4800, 0.042, 0.050, 0.016, 0.001, 0.5)
    print(f"  arbitrage band (lend 4.2%, borrow 5.0%, 10bp cost): "
          f"[{band.lower:.2f}, {band.upper:.2f}]")
    for months in (6, 4, 2, 1, 0):
        f = fu.cost_of_carry_price(4800, 0.045, 0.016, 0, months / 12)
        print(f"    {months:3d} months: F = {f:8.2f}, basis = "
              f"{fu.basis(4800, f):7.2f}")

    table.heading("20.4 A minimum-variance cross hedge")
    df = [
        0.021, -0.034, 0.012, 0.045, -0.018, -0.027, 0.009, 0.038, -0.041,
        0.016, -0.006, 0.029
    ]
    ds = [
        0.017, -0.031, 0.015, 0.036, -0.019, -0.020, 0.004, 0.035, -0.033,
        0.010, -0.002, 0.024
    ]
    fit = fu.minimum_variance_hedge(ds, df)
    table.value("hedge ratio", fit.beta)
    table.percent("variance removed at h*",
                  fu.hedge_effectiveness(ds, df, fit.beta))
    table.percent("variance removed at h = 1",
                  fu.hedge_effectiveness(ds, df, 1.0))
    table.value("contracts for 2.5m gallons (42,000/contract)",
                fu.contracts_to_hedge(fit.beta, 2.5e6, 42000))

    table.heading("20.4 Changing a portfolio's beta with index futures")
    for target in (0.0, 0.5, 1.0, 1.5):
        n = fu.index_futures_contracts(3.5e7, 1.15, target, 4850, 50)
        print(f"  target beta {target:.1f}: {n:+8.1f} contracts")

    table.heading("20.6 Options on futures (Black's model)")
    for k in (75.0, 80.0, 85.0, 90.0):
        c = fu.black_futures_option_price(OptionType.CALL, 82.5, k, 0.045, 0.32,
                                          0.25)
        p = fu.black_futures_option_price(OptionType.PUT, 82.5, k, 0.045, 0.32,
                                          0.25)
        print(f"  K {k:5.1f}: call {c:6.3f}  put {p:6.3f}")


if __name__ == "__main__":
    main()
