"""Worked examples for Chapter 8.

Run: python -m investm.ch08_riskfree_borrowing_lending.examples
"""

from investm.ch07_portfolio_analysis.examples import COV
from investm.ch07_portfolio_analysis.examples import MU
from investm.ch08_riskfree_borrowing_lending import riskfree as rfm
from investm.core import table


def main() -> None:
    table.heading("8.2-8.3 Riskfree lending and borrowing with one portfolio")
    print("  risky portfolio: E = 10%, sd = 18%; riskfree rate 4%")
    for x in (0.0, 0.25, 0.5, 0.75, 1.0, 1.25, 1.5):
        c = rfm.combine_with_riskfree(0.04, 0.10, 0.18, x)
        pos = "lending" if x < 1 else ("borrowing" if x > 1 else "all risky")
        print(f"  {x:8.2f} {100 * c.expected_return:8.2f} "
              f"{100 * c.std_dev:8.2f}  {pos}")

    table.heading("8.4 The tangency portfolio and the optimal portfolio O*")
    rf = 0.035
    t = rfm.tangency_portfolio(MU, COV, rf)
    tn = rfm.tangency_portfolio_no_short_sales(MU, COV, rf)
    for name, p in (("T, short sales", t), ("T, no short sales", tn)):
        w = " ".join(f"{x:+6.3f}" for x in p.weights)
        print(f"  {name:<20} E={100 * p.expected_return:6.2f}% "
              f"sd={100 * p.std_dev:6.2f}% Sharpe="
              f"{rfm.sharpe_ratio(p.expected_return, p.std_dev, rf):.4f}  "
              f"w: {w}")
    for tau in (0.1, 0.3, 0.6, 1.2):
        x = rfm.optimal_fraction_in_tangency(t, rf, tau)
        c = rfm.combine_with_riskfree(rf, t.expected_return, t.std_dev, x)
        print(f"  tau = {tau:.1f}: {100 * x:5.1f}% in T -> "
              f"E={100 * c.expected_return:5.2f}% sd={100 * c.std_dev:5.2f}%")

    table.heading("Appendix A Lend at 3.0%, borrow at 4.5%")
    for tau in (0.1, 0.3, 0.6, 1.0, 2.0):
        c = rfm.optimal_with_different_rates(MU, COV, 0.03, 0.045, tau)
        print(
            f"  tau = {tau:.1f}  {c.region.value:<10}  risky "
            f"{100 * c.fraction_risky:6.1f}%  E={100 * c.expected_return:5.2f}%"
            f" sd={100 * c.std_dev:5.2f}%")


if __name__ == "__main__":
    main()
