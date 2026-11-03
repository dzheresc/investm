# Comparison with open-source libraries: plan

This page picks the open-source libraries to compare investm against, and
the functions to compare. The results are in
[Comparison results](comparison.md); our own timings are in
[Performance](performance.md).

Each comparison gives two numbers on the same inputs and the same host: the
time per call, and the error against a reference value. A faster routine that
is less accurate, or a slower one that is more accurate, is not a
like-for-like result, so both are always reported.

## Candidates

Versions are the latest on PyPI (and, for the C++ QuantLib, in Ubuntu 24.04)
in November 2026.

| Library | Language | Licence | Overlap with investm |
| --- | --- | --- | --- |
| [QuantLib](https://www.quantlib.org) 1.33 (Ubuntu) / 1.44 (wheel) | C++, SWIG Python wheel | BSD-3 | Ch. 5, 13, 14 (bond pricing, yields, duration, day counts, curve bootstrapping), Ch. 19-20 (Black-Scholes, Greeks, implied volatility, binomial trees, Black's model), normal distribution, linear algebra |
| [numpy-financial](https://github.com/numpy/numpy-financial) 1.1 | Python/NumPy | BSD-3 | Ch. 5 and 24: present value, rate (yield), IRR (dollar-weighted return) |
| [SciPy](https://scipy.org) 1.18 | Python/C/Fortran | BSD-3 | Normal CDF and inverse, root finding (`brentq`), constrained optimization (`SLSQP`), `linregress` |
| [statsmodels](https://www.statsmodels.org) 0.15 | Python | BSD-3 | Ch. 4, 7, 24: OLS with standard errors (market model, timing regressions), runs test, autocorrelation |
| [PyPortfolioOpt](https://github.com/robertmartin8/PyPortfolioOpt) 1.6 | Python on cvxpy | MIT | Ch. 7-9: max-Sharpe, minimum-variance and quadratic-utility portfolios, CAPM-implied (Black-Litterman prior) returns |
| [cvxpy](https://www.cvxpy.org) 1.9 | Python modelling layer (Clarabel, OSQP) | Apache-2.0 | Ch. 24: style analysis as a QP |
| [arch](https://github.com/bashtage/arch) 8.0 | Python | NCSA | Ch. 4: variance-ratio test |
| [vollib](https://github.com/vollib/vollib) 1.0 | Python (Let's Be Rational) | MIT | Ch. 19: Black-Scholes price, analytic Greeks, implied volatility |
| [mpmath](https://mpmath.org) 1.4 | Python | BSD-3 | Not compared: computes the 50-digit references |

## Left out

- **Eigen, Armadillo, LAPACK.** investm's `core/matrix` is deliberately small
  (tens of assets). A BLAS-backed library wins at any size and measures
  something else; QuantLib's own matrix routines are the fairer peer, and
  NumPy already sits under every Python comparison.
- **riskfolio-lib, skfolio.** Wider portfolio toolkits (CVaR, risk parity,
  hierarchical clustering); for the book's mean-variance problems they solve
  the same cvxpy QP as PyPortfolioOpt.
- **FinancePy, ORE.** Derivatives-desk libraries; the book's option chapters
  are covered by QuantLib and vollib.
- **empyrical / quantstats.** Sharpe, Treynor and alpha are a few lines of
  arithmetic; there is nothing to time or get wrong beyond conventions
  (annualization, ddof), which differ by design.
- Chapters with no algorithmic counterpart in any library: order types and
  margin (Ch. 2), call auctions (Ch. 3), taxes (Ch. 12), corporate actions and
  earnings models (Ch. 16-18), investment companies (Ch. 21), ratio analysis
  (Ch. 22), CPPI and constant mix (Ch. 23), and currency parity (Ch. 25).

## Functions to compare

| investm function | Ch. | QuantLib | Python libraries | Reference for the error |
| --- | --- | --- | --- | --- |
| `BlackScholesPrice` | 19 | `blackFormula`, `AnalyticEuropeanEngine` | vollib `black_scholes` | Closed form in mpmath |
| `BlackScholesGreeks` | 19 | `BlackCalculator` | vollib analytic Greeks | Closed forms in mpmath |
| `ImpliedVolatility` | 19 | `blackFormulaImpliedStdDev` | vollib `implied_volatility` | mpmath root |
| `BinomialPrice` (American and European) | 19 | `BinomialVanillaEngine<CoxRossRubinstein>` | - | 20,000-step tree; Black-Scholes |
| `stats::NormCdf`, `ch06::NormInv` | 6, 19 | `CumulativeNormalDistribution`, `InverseCumulativeNormal` | SciPy `ndtr`, `ndtri`, `norm.cdf`, `norm.ppf` | mpmath |
| `PresentValueAtYield` | 5 | `BondFunctions::cleanPrice` | numpy-financial `pv`, `npv` | Exact rationals |
| `YieldToMaturity` | 5 | `BondFunctions::yield` | numpy-financial `rate`; SciPy `brentq` | mpmath root |
| `MacaulayDuration`, `ModifiedDuration`, `Convexity` | 14 | `BondFunctions::duration`, `convexity` | - | Exact rationals |
| `AccruedInterest`, `CleanPrice` (act/act, between coupons) | 13 | `FixedRateBond` with `ActualActual(ISMA)` | - | mpmath |
| `Bootstrap` | 5 | `PiecewiseYieldCurve<Discount, LogLinear>` on `FixedRateBondHelper` | - | The curve the prices came from |
| `InternalRateOfReturn` (dollar-weighted return) | 5, 24 | - | numpy-financial `irr`; SciPy `brentq` | mpmath root |
| `TangencyPortfolio` | 8 | - | PyPortfolioOpt `max_sharpe` | Closed form |
| `TangencyPortfolioNoShortSales` | 8 | - | PyPortfolioOpt `max_sharpe`; SciPy SLSQP | Best Sharpe ratio found |
| `OptimalNoShortSales` | 7 | - | PyPortfolioOpt `max_quadratic_utility`; SciPy SLSQP | Best objective found |
| `ImpliedExpectedReturns` | 9 | - | PyPortfolioOpt `market_implied_prior_returns` | mpmath |
| `StyleAnalysis` | 24 | - | cvxpy; SciPy SLSQP | Smallest tracking variance found |
| `FitMarketModel` | 7 | - | statsmodels `OLS`; SciPy `linregress`; NumPy `polyfit` | Exact rational least squares |
| `TreynorMazuy` | 24 | - | statsmodels `OLS` | Exact rational least squares |
| `RunsTest` | 4 | - | statsmodels `runstest_1samp` | mpmath |
| `VarianceRatio` | 4 | - | arch `VarianceRatio` | The book's formula in mpmath |
| `Autocorrelation` | 4 | - | statsmodels `acf`; pandas `autocorr` | mpmath |
| `EigenSymmetric`, `Cholesky`, `Inverse` | 7, 10 | `SymmetricSchurDecomposition`, `CholeskyDecomposition`, `inverse` | - | Residuals of the factorization |

## Method

- **Same host, same settings, one thread.** Both sides run back to back on
  one machine (the cloud container is noisy, see
  [Performance](performance.md)); `OMP_NUM_THREADS=1` and
  `OPENBLAS_NUM_THREADS=1` pin the thread pools.
- **C++ against C++:** the QuantLib cases are a separate Google Benchmark
  binary (`cpp/compare/`), built only when the `INVESTM_BUILD_COMPARE` CMake
  option is on, so the default build and CI tests do not depend on QuantLib.
  Each case reports its result as a counter in the JSON output, and an
  `error` counter where the reference can be computed in C++.
  Median of 5 repetitions.
- **Python against Python:** `scripts/compare/compare_python.py` times the
  investm Python package against the libraries above on the same cases,
  best of 5 `timeit` runs. The libraries go in
  `scripts/compare/requirements.txt`, never in investm's own dependencies.
- **References that are not another library.** Where possible the reference
  is exact (rational arithmetic with `fractions.Fraction`) or computed to 50
  digits with mpmath, so the error column is each library's own error. For
  optimizers the reference is the best objective any of them reached, and
  the error is the shortfall.
- **Settings matched, defaults shown.** Tolerances are matched where a
  library exposes them, and the library default is shown as well when it
  changes the answer (QuantLib's implied volatility, numpy-financial's
  `rate`).
- **Results** go in `docs/perf/compare-cpp.json`,
  `docs/perf/compare-python.json` and [Comparison results](comparison.md),
  with each library's version.
