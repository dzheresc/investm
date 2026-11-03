# Chapter 7: Portfolio Analysis

Code: [`cpp/ch07_portfolio_analysis`](../cpp/ch07_portfolio_analysis) and [`python/investm/ch07_portfolio_analysis`](../python/investm/ch07_portfolio_analysis).

| Section | Topic | C++ (`investm::ch07`) | Python |
|---|---|---|---|
| 7.1.1-7.1.2 | The feasible set and the efficient set theorem | `UnconstrainedFrontier` (short sales allowed), `EfficientSetNoShortSales` (`efficient_set.h`) | `efficient_set.*` |
| 7.1.3 | Selecting the optimal portfolio | `UnconstrainedFrontier::Optimal(tau)`, `OptimalNoShortSales(mu, cov, tau)` | |
| 7.2 | Concavity of the efficient set | `TwoAssetMinVarianceWeight`; the examples tabulate ρ = +1, 0 and −1 | |
| 7.3 | The market model, beta and residuals | `FitMarketModel`, `MarketModel` (`market_model.h`) | `market_model.*` |
| 7.4.1-7.4.4 | Total, market and unique risk | `PortfolioRiskFromMarketModels` | |
| 7.4.5 | Random versus efficient diversification | `RandomDiversificationVariance`, `AverageVarianceAndCovariance` | |
| App. A | The Markowitz model: composition of the efficient set and the optimal portfolio | `UnconstrainedFrontier`, `OptimalNoShortSales` | |
| App. B | Inputs needed to locate the efficient set | `MarketModelCovariance`, `MarketModelExpectedReturns` | |

## How the efficient set is computed

**With short sales.** Let A = 1′Σ⁻¹1, B = 1′Σ⁻¹μ, C = μ′Σ⁻¹μ and D = AC − B². The frontier portfolio with expected return E is

    w(E) = Σ⁻¹(λ·1 + γ·μ),   λ = (C − B·E)/D,   γ = (A·E − B)/D
    σ²(E) = (A·E² − 2B·E + C)/D

The investor with risk tolerance τ, who maximizes E − σ²/τ, holds

    w = (τ/2)·Σ⁻¹μ + ((1 − (τ/2)·B)/A)·Σ⁻¹1

**Without short sales.** Here the code solves `max τ·μ′w − w′Σw` subject to `w ≥ 0` and `Σw = 1`. The solver is accelerated projected gradient (FISTA), and each step projects exactly onto the simplex. The objective is convex, so the solver converges to the global optimum. The unit tests check the KKT conditions directly. Sweeping τ from 0 upward traces the efficient set from the minimum-variance portfolio up to the highest-return security, and you can watch securities enter and leave the optimal portfolio at the corner portfolios.
