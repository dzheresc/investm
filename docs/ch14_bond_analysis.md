# Chapter 14: Bond Analysis

Code: [`cpp/ch14_bond_analysis`](../cpp/ch14_bond_analysis) and [`python/investm/ch14_bond_analysis`](../python/investm/ch14_bond_analysis).

| Section | Topic | C++ (`investm::ch14`) | Python |
|---|---|---|---|
| 14.1 | Capitalization of income: price as PV of promised flows | See Ch. 5 `PresentValueAtYield`, `YieldToMaturity` | |
| 14.2 | Bond pricing theorems | Examples and the `PricingTheorems` test | |
| 14.3 | Macaulay and modified duration, PVBP | `MacaulayDuration`, `ModifiedDuration`, `DollarDuration`, `PriceValueOfBasisPoint` | `duration.*` |
| 14.3 | Convexity and the second-order price estimate | `Convexity`, `ApproxPriceChange` | |
| 14.3 | Effective duration and convexity for bonds with options | `EffectiveDuration`, `EffectiveConvexity` | |
| 14.4 | Promised vs expected yield, default premium | `ExpectedCashFlows`, `DecomposeYield` | `credit.*` |
| 14.4 | Default probability implied by a spread | `ImpliedDefaultProbability` | |
| 14.5 | Determinants of yield spreads | Examples: multiple regression with `investm::MultipleOls` | |
| 14.6 | Financial ratios as predictors of default | `LinearDiscriminant` (Fisher discriminant) | `credit.LinearDiscriminant` |

## Notes

- Convexity is (1/P) d²P/dy², computed in closed form for periodic compounding. Tests check it, and modified duration, against central finite differences.
- `DecomposeYield` splits the promised yield into the expected yield and a default premium. The expected yield is then split into the riskfree rate and a risk premium. The default model is simple: a constant annual default probability, and on default a recovery of a fixed fraction of face value.
- The discriminant uses the pooled within-group covariance. The cutoff is the midpoint of the two group means, which assumes equal priors and equal misclassification costs.
