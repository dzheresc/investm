# Chapter 15: Bond Portfolio Management

Code: [`cpp/ch15_bond_portfolio`](../cpp/ch15_bond_portfolio) and [`python/investm/ch15_bond_portfolio`](../python/investm/ch15_bond_portfolio).

| Section | Topic | C++ (`investm::ch15`) | Python |
|---|---|---|---|
| 15.2.1 | Horizon analysis and realized compound yield | `HorizonAnalysis` | `bond_portfolio.*` |
| 15.2.2 | Bond swaps (substitution, yield pickup) | Examples built on `HorizonAnalysis` | |
| 15.2.3 | Riding the yield curve | Examples | |
| 15.2.4 | Contingent immunization | `RequiredImmunizedAssets`, `ContingentCushion` | |
| 15.3 | Portfolio duration | `PortfolioDuration` | |
| 15.3.2 | Immunization | `ImmunizeTwoBonds`, `Surplus`, `Aggregate` | |
| 15.3.3 | Cash matching (dedication) | `CashMatch` | |
| 15.4 | Interest rate swaps | `ParSwapRate`, `PayFixedSwapValue` | |

## Notes

- Immunization matches the present value and the duration of assets to those of the liability. The examples and tests show the surplus as a function of a parallel shift. The surplus is zero at the starting yield and positive on both sides, because the asset portfolio is more convex than the single liability.
- `CashMatch` is the classic backward pass. It is simple and exact when one bond matures on each liability date. A minimum-cost dedicated portfolio over a wider universe is a linear program, which is out of scope here.
- Swap valuation uses the standard fact that, on a reset date, the floating leg is worth the notional times (1 − D_n).
