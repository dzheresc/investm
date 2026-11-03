# Chapter 11: Arbitrage Pricing Theory

Code: [`cpp/ch11_apt`](../cpp/ch11_apt) and [`python/investm/ch11_apt`](../python/investm/ch11_apt).

| Section | Topic | C++ (`investm::ch11`) | Python |
|---|---|---|---|
| 11.1.1-11.1.2 | The principle of arbitrage and arbitrage portfolios | `IsArbitragePortfolio`, `FindArbitragePortfolio` | `apt.*` |
| 11.2 | Pricing effects | Examples: buying the underpriced security pushes its return back onto the line | |
| 11.3-11.4 | The APT pricing equation and multiple factors | `FitAptPricing`, `AptExpectedReturn` | |
| 11.4 | Factor risk premiums | `PureFactorPortfolio` earns λ₀ + λ_k | |
| 11.5 | A synthesis of the APT and the CAPM | Examples: a single market factor turns the APT line into the SML | |
| 11.6 | Identifying the factors | See the Ch. 10 estimators | |

## How an arbitrage portfolio is found

An arbitrage portfolio is a set of trades X with ΣX = 0, B′X = 0 and μ′X > 0. To find one, the code projects μ onto the column space of [1 | B]. The residual is orthogonal to the constant vector and to every factor-sensitivity column, so it is self-financing and has no factor risk. Its expected return is μ′X = |X|², which is strictly positive unless μ lies exactly on the APT plane. In that case the function reports that no arbitrage exists.
