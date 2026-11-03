"""Worked examples for Chapter 4. All data are simulated.

Run: python -m investm.ch04_efficient_markets.examples
"""

import random

from investm.ch04_efficient_markets import efficiency_tests as et
from investm.ch04_efficient_markets import schedules
from investm.core import stats
from investm.core import table


def main() -> None:
    table.heading("4.1 Demand-to-buy and supply-to-sell schedules")
    inv = [
        schedules.Investor(48.0, 1000, 800),
        schedules.Investor(50.0, 600, 1000),
        schedules.Investor(52.0, 400, 900),
        schedules.Investor(55.0, 200, 1200),
        schedules.Investor(45.0, 1500, 500),
        schedules.Investor(58.0, 0, 700),
    ]
    rows = [(float(p), schedules.demand_to_buy(inv, p, 5.0),
             schedules.supply_to_sell(inv, p, 5.0)) for p in range(46, 58)]
    table.table(["price", "demand", "supply"], rows, precision=1)
    p = schedules.equilibrium_price(inv)
    table.value("equilibrium price", p, "$")
    table.value("volume at equilibrium", schedules.demand_to_buy(inv, p, 5.0),
                "sh")
    print("  4.1.5 Good news raises every valuation by 5%:")
    for i in inv:
        i.value *= 1.05
    table.value("new equilibrium price", schedules.equilibrium_price(inv), "$")

    table.heading("4.2.3 / 4.3.2 Weak-form tests on a simulated random walk")
    prices = et.random_walk_prices(100.0, 2000, 0.0003, 0.012, 42)
    r = et.log_returns(prices)
    table.value("lag-1 autocorrelation", stats.autocorrelation(r, 1))
    runs = et.runs_test(r)
    print(f"  runs: {runs.runs} observed vs {runs.expected_runs:.1f} "
          f"expected, z = {runs.z:.3f}")
    for q in (2, 5, 10, 20):
        print(f"  variance ratio VR({q:2d}) = {et.variance_ratio(r, q):.4f}")
    table.percent("buy-and-hold return", prices[-1] / prices[0] - 1.0)
    for x in (0.01, 0.05, 0.10):
        table.percent(f"{100 * x:.0f}% filter rule return",
                      et.filter_rule_return(prices, x))

    table.heading("4.3.1 Event study: 30 firms, good news on day 0")
    rng = random.Random(7)
    estimation, event = 120, 11
    firms = []
    for f in range(30):
        beta = 0.7 + 0.02 * f
        m = [rng.gauss(0.0004, 0.01) for _ in range(estimation + event)]
        s = [0.0001 + beta * x + rng.gauss(0.0, 0.015) for x in m]
        s[estimation + 5] += 0.03
        firms.append(et.EventFirm(s, m))
    res = et.event_study(firms, estimation)
    rows = [(float(i - 5), 100 * a, 100 * c)
            for i, (a, c) in enumerate(zip(res.aar, res.caar))]
    table.table(["day", "AAR %", "CAAR %"], rows, precision=3)
    table.value("t statistic of final CAAR", res.caar_t_stat)


if __name__ == "__main__":
    main()
