# Chapter 4: Efficient Markets, Investment Value, and Market Price

Code: [`cpp/ch04_efficient_markets`](../cpp/ch04_efficient_markets) and [`python/investm/ch04_efficient_markets`](../python/investm/ch04_efficient_markets).

| Section | Topic | C++ (`investm::ch04`) | Python |
|---|---|---|---|
| 4.1.1-4.1.3 | Demand and supply schedules and how they interact | `Investor`, `DemandToBuy`, `SupplyToSell`, `EquilibriumPrice` (`schedules.h`) | `schedules.*` |
| 4.1.4 | Elasticity | `ArcElasticity` | `schedules.arc_elasticity` |
| 4.1.5 | Shifts of the schedules | Examples program: every valuation rises 5% and the price follows | |
| 4.2.3 | Random walk | `RandomWalkPrices`, `LogReturns` (`efficiency_tests.h`) | `efficiency_tests.random_walk_prices` |
| 4.3.1 | Event studies | `EventStudy` (market-model abnormal returns, AAR and CAAR, t statistic) | `efficiency_tests.event_study` |
| 4.3.2 | Looking for patterns | `RunsTest`, `VarianceRatio`, `FilterRuleReturn`, `stats::Autocorrelation` | same names in snake_case |
| 4.3.3 | Examining performance | Covered properly in Ch. 24. | |
| 4.4 | Summary of the evidence | The tests above make up the weak-form and semistrong-form toolkit. | |

## Notes

- **Schedules.** Each investor has a private estimate of the stock's value. Investors who think the stock is cheap at price *P* create demand, and holders who think it is dear create supply. When new information changes the valuations, both schedules shift and the clearing price moves. In an efficient market that adjustment happens quickly.
- **Event study.** The market model is fitted on a clean estimation window. Abnormal returns are then measured over the event window and averaged across firms, so the firm-specific noise cancels out.
- **Variance ratio.** Under a random walk, the variance of a *q*-period return is *q* times the one-period variance, so VR(q) should be close to 1.
