# Chapter 23: Investment Management

Code: [`cpp/ch23_investment_management`](../cpp/ch23_investment_management) and [`python/investm/ch23_investment_management`](../python/investm/ch23_investment_management).

| Section | Topic | C++ (`investm::ch23`) | Python |
|---|---|---|---|
| 23.3 | Estimating risk tolerance from a chosen mix | `ImpliedRiskTolerance`, `OptimalStockWeight` | `management.*` |
| 23.4 | Strategic asset allocation (mean-variance, no short sales) | Examples using `ch07::OptimalNoShortSales` | |
| 23.4 | Buy-and-hold | `BuyAndHold` | |
| 23.4 | Constant mix, with an optional rebalancing band | `ConstantMix` | |
| 23.4 | Constant-proportion portfolio insurance | `Cppi` | |

## Notes

- Risk tolerance uses the Chapter 6 utility U = E − σ²/τ, so a holding of w in stocks implies τ = 2σ²w/(E − r_f).
- Constant mix buys after falls and sells after rises. It wins in markets that reverse and loses in markets that trend. CPPI does the opposite: it gives a convex payoff with a floor. The Monte Carlo example shows these trade-offs in the tails of terminal wealth.
