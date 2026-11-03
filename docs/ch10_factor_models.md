# Chapter 10: Factor Models

Code: [`cpp/ch10_factor_models`](../cpp/ch10_factor_models) and [`python/investm/ch10_factor_models`](../python/investm/ch10_factor_models).

| Section | Topic | C++ (`investm::ch10`) | Python |
|---|---|---|---|
| 10.1 | Factor models and return-generating processes | `FactorModel` (a, B, residual variances) | `factor_models.FactorModel` |
| 10.2 | One-factor models, including the market model | `FactorModel` with K = 1; see also Ch. 7 | |
| 10.3.1-10.3.3 | Two-factor, sector-factor and extended models | `FactorExpectedReturns`, `FactorCovariance`, `PortfolioSensitivities`, `DecomposePortfolioVariance`, `SectorDummies` | |
| 10.4.1 | Time-series approaches | `EstimateTimeSeries` | |
| 10.4.2 | Cross-sectional approaches | `EstimateCrossSectional` (zero factor optional) | |
| 10.4.3 | Factor-analytic approaches | `EstimatePrincipalComponents` (Jacobi eigen-decomposition in C++, `numpy.linalg.eigh` in Python) | |
| 10.5 | Factor models and equilibrium | Continued in Ch. 11 (APT) | |

## Notes

A K-factor model reduces the inputs for N securities from N(N+1)/2 covariances to NK + N + K(K+1)/2 numbers. For 500 stocks and two factors, that is 125,250 estimates versus 1,503. The examples print this comparison.

The shared `cpp/core/matrix.h` and `cpp/core/regression.h` were added for this part of the book: a dense matrix with LU, Cholesky and Jacobi eigen-decomposition, and OLS with standard errors.
