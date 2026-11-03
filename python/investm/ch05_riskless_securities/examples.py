"""Worked examples for Chapter 5.

Run: python -m investm.ch05_riskless_securities.examples
"""

from investm.ch05_riskless_securities import cash_flows as cf
from investm.ch05_riskless_securities import rates
from investm.ch05_riskless_securities import term_structure as ts
from investm.core import table


def main() -> None:
    table.heading("5.1 Nominal vs. real rates")
    table.percent("nominal 7%, inflation 3%: exact real rate",
                  rates.real_rate(0.07, 0.03))
    table.percent("approximation nominal - inflation",
                  rates.real_rate_approx(0.07, 0.03))

    table.heading("5.2 YTM of a 4-year 6% annual-coupon bond at $965")
    bond = cf.coupon_bond(1000, 0.06, 4)
    y = cf.yield_to_maturity(bond, 965.0)
    table.percent("YTM", y)
    table.percent(
        "same bond paying semiannually (APR, m=2)",
        cf.yield_to_maturity(cf.coupon_bond(1000, 0.06, 4, 2), 965.0, 2))

    table.heading("5.3-5.6, 5.9 Spot rates, discount factors, forwards")
    quotes = [
        ts.BondQuote(1000, 0.050, 1001.90),
        ts.BondQuote(1000, 0.055, 999.10),
        ts.BondQuote(1000, 0.060, 994.80),
        ts.BondQuote(1000, 0.065, 993.40),
        ts.BondQuote(1000, 0.065, 979.70),
    ]
    curve = ts.bootstrap(quotes)
    rows = [(t, quotes[t - 1].price, 100 * curve.spot(t),
             curve.discount_factor(t), 100 * curve.one_year_forward(t),
             100 * curve.par_yield(t))
            for t in range(1, curve.max_maturity + 1)]
    table.table(["t", "price", "spot %", "d_t", "f(t-1,t) %", "par %"], rows)
    table.percent("2-year rate starting in 3 years, f(3,5)",
                  curve.forward(3, 5))

    table.heading("5.2 vs. 5.3 YTM is not the right discount rate")
    b5 = cf.coupon_bond(1000, 0.08, 5)
    fair = curve.present_value(b5)
    table.value("5-year 8% bond valued off the spot curve", fair, "$")
    table.percent("its YTM", cf.yield_to_maturity(b5, fair))
    table.percent("5-year spot rate", curve.spot(5))

    table.heading("5.7 Compounding an 8% quoted annual rate")
    for m in (1, 2, 4, 12, 365):
        print(f"  m = {m:3d}     effective annual "
              f"{100 * rates.effective_annual_rate(0.08, m):.6f}%")
    print(f"  continuous  effective annual "
          f"{100 * rates.effective_from_continuous(0.08):.6f}%")

    table.heading("5.8 A 91-day T-bill quoted at a 4.80% discount")
    p = rates.bill_price_from_discount(10000, 0.048, 91)
    table.value("price", p, "$")
    table.percent("bank discount yield",
                  rates.bank_discount_yield(10000, p, 91))
    table.percent("bond-equivalent yield",
                  rates.bond_equivalent_yield(10000, p, 91))
    table.percent("money-market yield", rates.money_market_yield(10000, p, 91))
    table.percent("effective annual yield",
                  rates.bill_effective_annual_yield(10000, p, 91))

    table.heading("5.10 Term structure theories")
    print("  unbiased expectations: expected 1-year rates " + " ".join(
        f"{100 * e:.2f}%" for e in ts.expected_future_one_year_rates(curve)))
    lp = ts.liquidity_preference_curve([0.05] * 5,
                                       [0.0, 0.004, 0.007, 0.009, 0.010])
    print("  liquidity preference, flat 5% expectations, rising premiums:\n"
          "    " + " ".join(f"{100 * s:.3f}%" for s in lp.rates))

    table.heading("5.11 One-year holding-period returns on zeros")
    nxt = ts.roll_forward_one_year(curve)
    flat = ts.SpotCurve([0.06] * 4)
    rows = [(n, 100 * ts.one_year_hpr_of_zero(curve, nxt, n),
             100 * ts.one_year_hpr_of_zero(curve, flat, n))
            for n in range(1, curve.max_maturity + 1)]
    table.table(["maturity", "HPR fwd %", "HPR flat %"], rows)


if __name__ == "__main__":
    main()
