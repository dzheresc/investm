"""Worked examples for Chapter 19.

Run: python -m investm.ch19_options.examples
"""

from investm.ch19_options import payoffs as po
from investm.ch19_options import pricing as pr
from investm.core import table

CALL = po.OptionType.CALL
PUT = po.OptionType.PUT
I = po.Instrument


def main() -> None:
    table.heading("19.2 Profit at expiry of some option strategies")
    strategies = {
        "bull call": [
            po.Leg(I.CALL, 45, 1, 4.10),
            po.Leg(I.CALL, 55, -1, 1.20)
        ],
        "straddle": [po.Leg(I.CALL, 50, 1, 2.60),
                     po.Leg(I.PUT, 50, 1, 2.25)],
        "protective": [po.Leg(I.STOCK, 0, 1, 50.0),
                       po.Leg(I.PUT, 45, 1, 0.90)],
        "covered": [po.Leg(I.STOCK, 0, 1, 50.0),
                    po.Leg(I.CALL, 55, -1, 1.20)],
    }
    table.table(["S_T"] + list(strategies),
                [[float(s)] +
                 [po.strategy_profit(legs, s)
                  for legs in strategies.values()]
                 for s in range(35, 66, 5)],
                precision=2)

    table.heading("19.3 Put-call parity (S 62, K 60, r 4.5%, 4 months)")
    t = 4 / 12
    table.value("put implied by a $5.10 call",
                po.parity_put_price(5.10, 62, 60, 0.045, t))
    table.value("parity deviation if the put trades at $2.50",
                po.parity_deviation(5.10, 2.50, 62, 60, 0.045, t))
    table.value("call lower bound", po.call_lower_bound(62, 60, 0.045, t))

    table.heading("19.4 Binomial model")
    r = pr.one_period_binomial(CALL, 80, 80, 1.25, 0.85, 0.03)
    print("  one period, S 80 -> 100 or 68, K 80, r 3%:")
    print(f"  buy {r.shares:.4f} shares, borrow {-r.bond:.4f} -> call worth "
          f"{r.value:.4f}")
    put = pr.OptionSpec(PUT, 48, 50, 0.05, 0.32, 0.5)
    table.table(
        ["steps", "European", "American"],
        [[n,
          pr.binomial_price(put, n, False),
          pr.binomial_price(put, n, True)] for n in (1, 2, 5, 20, 100, 500)],
        precision=5)
    table.value("Black-Scholes European put", pr.black_scholes_price(put))

    table.heading("19.5 Black-Scholes and the Greeks (S 105, r 4%, sigma 28%)")
    rows = []
    for kind in (CALL, PUT):
        for k in (95.0, 105.0, 115.0):
            g = pr.black_scholes_greeks(
                pr.OptionSpec(kind, 105, k, 0.04, 0.28, 0.5, 0.015))
            rows.append([
                kind.value, k, g.price, g.delta, g.gamma, g.vega, g.theta, g.rho
            ])
    table.table(
        ["type", "K", "price", "delta", "gamma", "vega", "theta", "rho"],
        rows,
        width=9)

    table.heading("19.5.4 Implied and historical volatility")
    s = pr.OptionSpec(CALL, 73, 75, 0.042, 0.3, 0.25)
    for price in (2.50, 3.20, 4.40):
        print(f"  call at {price:.2f} -> implied vol "
              f"{100 * pr.implied_volatility(s, price):6.2f}%")
    closes = [73.1, 74.0, 72.6, 73.9, 75.2, 74.4, 75.8, 76.1, 74.9, 75.5, 77.0]
    table.percent("historical vol from 10 daily closes",
                  pr.historical_volatility(closes, 252))

    table.heading("19.7 Warrants: 10m shares, 2m warrants at K 30, 3 years")
    w = pr.OptionSpec(CALL, 27, 30, 0.04, 0.35, 3.0)
    table.value("equivalent call", pr.black_scholes_price(w))
    table.value("warrant (dilution-adjusted)", pr.warrant_value(w, 1e7, 2e6))


if __name__ == "__main__":
    main()
