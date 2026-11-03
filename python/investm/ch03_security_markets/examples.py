"""Worked examples for Chapter 3.

Run: python -m investm.ch03_security_markets.examples
"""

from investm.ch03_security_markets import call_market
from investm.ch03_security_markets import costs
from investm.ch03_security_markets.call_market import LimitOrder
from investm.core import table


def main() -> None:
    table.heading("3.1.1 Clearing a call market")
    book = [
        LimitOrder(True, 30.50, 300),
        LimitOrder(True, 30.25, 500),
        LimitOrder(True, 30.00, 400),
        LimitOrder(True, 29.75, 600),
        LimitOrder(False, 29.50, 200),
        LimitOrder(False, 29.75, 300),
        LimitOrder(False, 30.00, 500),
        LimitOrder(False, 30.25, 700),
    ]
    rows = []
    for p in (29.50, 29.75, 30.00, 30.25, 30.50):
        d, s = call_market.demand_at(book, p), call_market.supply_at(book, p)
        rows.append((p, float(d), float(s), float(min(d, s))))
    table.table(["price", "demand", "supply", "volume"], rows, precision=2)
    r = call_market.clear_call_market(book)
    print(f"  clearing price {r.price:.2f}, volume {r.volume:.0f}, "
          f"imbalance {r.imbalance:+.0f}")

    table.heading("3.7 Commissions: full-service vs. discount broker")
    full = costs.CommissionSchedule(25.0, 0.0, 0.012, 40.0)
    discount = costs.CommissionSchedule(4.95, 0.005)
    for shares in (100, 1000, 10000):
        print(f"  {shares:6d} sh @ $40: full-service "
              f"${full.commission(shares, 40.0):8.2f}   discount "
              f"${discount.commission(shares, 40.0):7.2f}")

    table.heading("3.8 Transactions costs of a 50,000-share buy")
    quote = costs.Quote(39.98, 40.02)
    impact = costs.sqrt_price_impact(50000, 2_000_000, 0.02, 0.8)
    table.percent("quoted relative spread", quote.relative_spread)
    table.percent("estimated price impact", impact)
    table.percent("total one-way cost (discount broker)",
                  costs.one_way_cost(50000, 40.0, quote, discount, impact))


if __name__ == "__main__":
    main()
