# Chapter 16: Common Stocks

Code: [`cpp/ch16_common_stocks`](../cpp/ch16_common_stocks) and [`python/investm/ch16_common_stocks`](../python/investm/ch16_common_stocks).

| Section | Topic | C++ (`investm::ch16`) | Python |
|---|---|---|---|
| 16.1 | Majority and cumulative voting | `CumulativeVotingSharesNeeded`, `MajorityVotingSharesNeeded` | `corporate.*` |
| 16.3 | Stock splits and stock dividends; split-adjusted price series | `PriceAfterSplit`, `SplitRatioFromStockDividend`, `SplitAdjustPrices` | |
| 16.4 | Preemptive rights: ex-rights price and value of a right | `ExRightsPrice`, `RightValueCum`, `RightValueEx` | |
| 16.7 | Ex-post betas and rolling betas | `EstimateBeta`, `RollingBetas` | `betas.*` |
| 16.7 | Adjusting betas: mechanical and Bayesian | `ShrinkBeta`, `VasicekBeta`, `BetaPersistence` | |
| 16.7 | Leverage and beta | `LeveredBeta`, `UnleveredBeta` | |

## Notes

- Cumulative voting lets a minority holding just over S·n/(N+1) shares elect n of N directors. Under straight voting, the same group elects no one.
- Rights math: with N rights plus S to buy one new share, a right is worth (P_cum − S)/(N+1) = (P_ex − S)/N. A shareholder's wealth is unchanged by the offering.
- Betas regress toward one over time. `BetaPersistence` measures this with a cross-sectional regression of later betas on earlier ones. `ShrinkBeta` (2/3 raw + 1/3 × 1.0) and `VasicekBeta` (precision-weighted with a prior) are two standard corrections.
