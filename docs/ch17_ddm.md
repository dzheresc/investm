# Chapter 17: The Valuation of Common Stocks

Code: [`cpp/ch17_ddm`](../cpp/ch17_ddm) and [`python/investm/ch17_ddm`](../python/investm/ch17_ddm).

| Section | Topic | C++ (`investm::ch17`) | Python |
|---|---|---|---|
| 17.2 | Capitalization of income; NPV and IRR of buying a stock | `PresentValueOfDividends`, `NetPresentValue` | `ddm.*` |
| 17.3 | Zero-growth model | `ZeroGrowthValue` | |
| 17.4 | Constant-growth model and its implied return | `ConstantGrowthValue`, `ConstantGrowthImpliedReturn` | |
| 17.5 | Multiple-growth model and its implied return | `MultipleGrowthValue`, `MultipleGrowthImpliedReturn` | |
| 17.6 | Three-phase model (growth fades linearly) | `ThreePhaseDividends`, `ThreePhaseValue` | |
| 17.7 | Models based on P/E ratios | `ForwardPriceEarnings`, `TrailingPriceEarnings` | |
| 17.8 | Sources of earnings growth; PVGO | `SustainableGrowth`, `ValueWithReinvestment`, `PresentValueOfGrowthOpportunities` | |

## Notes

- With the same growth rate in every phase, the multiple-growth and three-phase models reduce to the constant-growth model. Tests check this.
- The implied return of the multiple-growth model is found by root finding on k in (g, ∞). Over that range the value falls monotonically from +∞.
- Retention adds value only when ROE exceeds k. The P/E table in the examples shows this: with ROE = k, the price is the same whatever the retention ratio, and PVGO is zero.
