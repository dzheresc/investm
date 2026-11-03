# Chapter 24: Portfolio Performance Evaluation

Code: [`cpp/ch24_performance`](../cpp/ch24_performance) and [`python/investm/ch24_performance`](../python/investm/ch24_performance).

| Section | Topic | C++ (`investm::ch24`) | Python |
|---|---|---|---|
| 24.2 | Time-weighted vs dollar-weighted returns | `PeriodReturnsFromValues`, `TimeWeightedReturn`, `DollarWeightedReturn` | `performance.*` |
| 24.4 | Sharpe, Treynor, Jensen's alpha, M², appraisal ratio | `Evaluate` | |
| 24.4 | Information ratio | `InformationRatio` | |
| 24.6 | Market timing: Treynor-Mazuy and Henriksson-Merton | `TreynorMazuy`, `HenrikssonMerton` | |
| 24.7 | Performance attribution (Brinson) | `BrinsonAttribution` | |
| 24.8 | Returns-based style analysis | `StyleAnalysis` | |

## Notes

- The time-weighted return measures the manager. The dollar-weighted return (an IRR) also reflects when the client added or withdrew money.
- Allocation, selection and interaction effects sum exactly to the active return. Tests check this.
- Style analysis minimizes the variance of the tracking error with weights that are non-negative and sum to one. It is solved by accelerated projected gradient, using the simplex projection from Chapter 7.
