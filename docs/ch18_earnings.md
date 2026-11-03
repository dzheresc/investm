# Chapter 18: Earnings

Code: [`cpp/ch18_earnings`](../cpp/ch18_earnings) and [`python/investm/ch18_earnings`](../python/investm/ch18_earnings).

| Section | Topic | C++ (`investm::ch18`) | Python |
|---|---|---|---|
| 18.1-18.2 | Earnings, dividends, payout and retention | See Ch. 17 `SustainableGrowth` | |
| 18.3 | Lintner's partial adjustment model of dividends | `LintnerModel`, `FitLintner` | `earnings.*` |
| 18.4 | Naive forecasts: random walk with drift, seasonal random walk | `RandomWalkForecast`, `SeasonalRandomWalkForecast` | |
| 18.5 | Earnings surprises: standardized unexpected earnings | `StandardizedUnexpectedEarnings` | |
| 18.5 | Returns sorted by surprise (post-announcement drift) | `SortIntoGroups`, `GroupMeans` | |

## Notes

- `FitLintner` regresses ΔD_t on E_t and D_{t−1}. The speed of adjustment c is minus the slope on D_{t−1}, and the target payout is the slope on E_t divided by c.
- The SUE uses the seasonal random walk with drift as the expectation, scaled by the standard deviation of past seasonal differences.
- The drift example uses a simulated panel with a fixed seed, so the C++ and Python numbers differ (different random generators) but tell the same story.
