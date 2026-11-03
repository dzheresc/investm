# Chapter 22: Financial Analysis

Code: [`cpp/ch22_financial_analysis`](../cpp/ch22_financial_analysis) and [`python/investm/ch22_financial_analysis`](../python/investm/ch22_financial_analysis).

| Section | Topic | C++ (`investm::ch22`) | Python |
|---|---|---|---|
| 22.3 | Profitability, liquidity, leverage and market ratios | `Financials`, `ComputeRatios` | `ratios.*` |
| 22.3 | Five-factor DuPont decomposition of ROE | `DecomposeRoe` | |
| 22.4 | Technical analysis: moving averages and crossover signals | `SimpleMovingAverage`, `MovingAverageSignal` | `technical.*` |
| 22.4 | Momentum and relative strength | `Momentum`, `RelativeStrength` | |
| 22.4 | Backtesting a moving-average timing rule | `MovingAverageRuleReturns` | |

## Notes

- The ratio example holds the business fixed and changes only the capital structure. Operating margin and turnover stay the same, while leverage raises the equity multiplier and lowers the interest burden. ROE rises and ROA falls.
- The technical-rule backtest runs on a simulated random walk. Any edge it shows is sampling noise, which is the point of the example.
