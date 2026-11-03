# Chapter 8: Riskfree Borrowing and Lending

Code: [`cpp/ch08_riskfree_borrowing_lending`](../cpp/ch08_riskfree_borrowing_lending) and [`python/investm/ch08_riskfree_borrowing_lending`](../python/investm/ch08_riskfree_borrowing_lending).

| Section | Topic | C++ (`investm::ch08`) | Python |
|---|---|---|---|
| 8.1 | Defining the riskfree asset | A rate `rf` with zero variance and zero covariance with everything else | |
| 8.2.1-8.2.2 | Investing in the riskfree asset and a risky asset or portfolio | `CombineWithRiskfree` with `x_risky` ≤ 1 | `riskfree.combine_with_riskfree` |
| 8.2.3, 8.4.1 | Effect on the efficient set | `TangencyPortfolio`, `TangencyPortfolioNoShortSales`, `SharpeRatio` | |
| 8.3 | Riskfree borrowing | `CombineWithRiskfree` with `x_risky` > 1 | |
| 8.2.4, 8.4.2 | Effect on portfolio selection | `OptimalFractionInTangency` | |
| App. A | Different borrowing and lending rates | `OptimalWithDifferentRates` returns the region (lend, risky only, or borrow) | |
| App. B | Composition of the optimal portfolio O* | `OptimalFractionInTangency` times the tangency weights | |

## Notes

- With short sales allowed, the tangency portfolio is proportional to Σ⁻¹(μ − rf·1). A tangency on the efficient (upper) branch exists only when rf is below the expected return of the minimum-variance portfolio, B/A. `TangencyPortfolio` returns `nullopt` / `None` otherwise.
- Without short sales, the code maximizes the Sharpe ratio along the no-short-sales efficient set with a golden-section search over the risk tolerance that indexes that set. This works because the slope from rf to a point on a concave frontier is unimodal.
- An investor with risk tolerance τ puts the fraction x = τ(E_T − rf)/(2σ_T²) of wealth in T. When x > 1, the investor borrows.
