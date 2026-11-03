# Chapter 6: The Portfolio Selection Problem

Code: [`cpp/ch06_portfolio_selection`](../cpp/ch06_portfolio_selection) and [`python/investm/ch06_portfolio_selection`](../python/investm/ch06_portfolio_selection).

| Section | Topic | C++ (`investm::ch06`) | Python |
|---|---|---|---|
| 6.1 | Initial and terminal wealth; the return on a portfolio | `WeightsFromHoldings`, `PortfolioReturnFromPrices` (`portfolio.h`) | `portfolio.*` |
| 6.2 | Nonsatiation and risk aversion | `Utility`: every kind is increasing; its curvature sets the risk attitude (`utility.h`) | `utility.Utility` |
| 6.3.1 | Marginal utility | `Utility::Marginal`, `Utility::AbsoluteRiskAversion` | |
| 6.3.2 | Certainty equivalents and risk premiums | `CertaintyEquivalent`, `RiskPremium` | |
| 6.4 | Indifference curves | `MeanVarianceUtility`, `IndifferenceCurve` | |
| 6.5.1 | Expected returns of portfolios | `PortfolioExpectedReturn` | |
| 6.5.2 | Standard deviations of portfolios | `PortfolioVariance`, `PortfolioStdDev`, `TwoAssetStdDev` | |
| App. A | Probability distributions: central tendency, dispersion, comovement | `MomentsFromScenarios`, `ScenarioPortfolioReturns`, plus `core/stats` | `portfolio.moments_from_scenarios` |
| App. B | Risk-neutral and risk-seeking investors | `UtilityKind::kLinear`, `UtilityKind::kConvex` | |
| Inst. issues | Alternative risk measures | `SemiVariance`, `DownsideDeviation`, `ShortfallProbability`, `HistoricalVaR`, `HistoricalExpectedShortfall`, `NormalVaR`, `NormalExpectedShortfall` (`risk_measures.h`) | `risk_measures.*` |

## Notes

- **Mean-variance indifference curves.** The curves use the utility U = E − σ²/τ, where τ is the investor's risk tolerance. Every indifference curve is then an upward-sloping parabola E = U + σ²/τ. The same τ also parameterizes the optimal portfolio in Chapters 7 and 8.
- **Risk premium.** The risk premium is expected wealth minus the certainty equivalent. It is positive for concave utility, zero for linear utility, and negative for convex utility, which is exactly the risk-averse, risk-neutral and risk-seeking split described in Appendix B.
- **Downside risk.** The risk-measures example draws returns from a fat-tailed distribution. On those returns, the normal approximation understates both VaR and expected shortfall.
