# Comparison with open-source libraries

This page has the results of timing investm against the libraries picked in
[the comparison plan](comparison_plan.md). Each case computes the same
quantity from the same inputs in every library that covers it, with
tolerances and step counts matched as far as each library allows. The tables
report the time per call and the error against a reference value, because a
faster result that is less accurate is not a like-for-like win. References
are exact (rational arithmetic) or computed to 50 digits with mpmath, so the
error column is each library's own error, not a difference between two
libraries.

Everything here ran in the cloud container.

## Summary

- **Every investm result agrees with its reference to 4e-14 or better,**
  in both languages (the binomial trees apart, whose discretization error
  matches QuantLib's). The comparison found no bug in investm.
- **C++ against QuantLib: investm is faster in all but two cases.** Bond
  price, yield, duration and accrued interest are 20-270x faster: investm
  works on a vector of (time, amount) pairs, QuantLib on its general leg of
  dated coupon objects and a day counter. The American CRR
  tree is 21x faster at 1,000 steps (QuantLib's generic lattice framework
  against investm's single array), and the European tree is 109x faster
  because investm sums the terminal payoffs in O(n) instead of rolling back.
  The bond bootstrap is 2,200x faster: investm solves one linear equation
  per maturity, QuantLib runs an iterative piecewise-curve bootstrap.
- **The two exceptions are small functions.** QuantLib's `blackFormula` is
  1.2x faster than `BlackScholesPrice` (44 against 53 ns; investm's price is
  `BlackScholesGreeks(...).price`, so it also computes the Greeks). QuantLib's
  `InverseCumulativeNormal` is 4.3x faster than `ch06::NormInv` (15 against
  64 ns), but it is the Acklam approximation without investm's Newton
  refinement, so its error is 5.6e-9 against investm's 1.3e-15.
- **Python: investm's NumPy code holds its own.** The no-short-sales
  optimizers are the fastest of PyPortfolioOpt (cvxpy), SciPy SLSQP and
  investm at 100 assets (2.2x faster than PyPortfolioOpt for the max-Sharpe
  portfolio, 4.2x for the quadratic-utility one, 23-42x faster than SLSQP),
  and every optimizer reaches the same optimum to 1e-15, except SLSQP on the
  100-asset max-Sharpe problem (2.2e-10 short).
- **Where investm's Python is slower, it is a pure-Python loop:** implied
  volatility (70 us against vollib's 15 us and QuantLib's 2.2 us), the
  act/act clean price (65 us against QuantLib's 8.8 us), the variance ratio
  (0.87 ms against arch's 0.37 ms), the autocorrelation (0.29 ms against
  statsmodels' 62 us) and the 100-step CRR tree (0.48 ms against
  QuantLib's 68 us). See [Follow-ups](#follow-ups).

Accuracy findings in the other libraries:

1. **QuantLib's `CumulativeNormalDistribution` has a relative error of
   1.3e-7 in the far left tail** (x near -8; the absolute error is 1.2e-15).
   For p-values and tail probabilities that is 7 significant digits. investm
   and SciPy use `erfc` and are good to 9e-15 relative.
2. **QuantLib's `InverseCumulativeNormal` is good to 5.6e-9 in z** (Acklam's
   approximation). investm (Acklam plus one Newton step in C++, Wichura's
   AS241 in Python) and SciPy are good to 3e-15.
3. **Default tolerances cost digits.** QuantLib's implied volatility at its
   default accuracy (1e-6) is off by 5.1e-7; numpy-financial's `rate` at its
   default tolerance (1e-6) gives a yield off by 1.2e-12. Both are exact to
   rounding when asked for 1e-12.
4. **QuantLib 1.33's `BondFunctions::yield` takes 0.49 ms in C++, while the
   1.44 wheel takes 81 us from Python** for the same bond, so the
   difference is the version, not the binding. It was not investigated
   further.
5. **Definitions differ in two places, by design:** arch's variance ratio is
   Lo and MacKinlay's (drift from the end points, divisor n, with or without
   their bias correction), 0.003 away from the book's ratio of sample
   variances. pandas' `autocorr` is the Pearson correlation of x[1:] and
   x[:-1], 2.4e-4 away from the textbook (full-sample) autocorrelation,
   which investm and statsmodels compute.

## Setup

| | |
| --- | --- |
| Host | Cloud container, 4 cores at 2.1 GHz, Ubuntu 24.04 (noisy; see [Performance](performance.md)) |
| C++ | GCC 13.3, `Release` (-O3), Google Benchmark 1.8.3, median of 5 repetitions |
| QuantLib (C++) | 1.33, Ubuntu 24.04 package `libquantlib0-dev` |
| Python | 3.13, NumPy 2.5.3, best of 5 `timeit` runs |
| Python libraries | QuantLib 1.44 (wheel), numpy-financial 1.1.0, SciPy 1.18.1, statsmodels 0.15.0, PyPortfolioOpt 1.6.0, cvxpy 1.9.3 (Clarabel), arch 8.0.0, vollib 1.0.11, pandas 3.0.5, mpmath 1.4.1 |
| Threads | One everywhere (`OMP_NUM_THREADS=1`, `OPENBLAS_NUM_THREADS=1`) |

QuantLib works with dates. A 30/360 day count on the 15th of the month makes
T = 5/12, 0.5, 1 and 10 exact, so its year fractions match investm's. The
act/act Treasury case uses `ActualActual(ISMA)` with the bond's own coupon
schedule, which is the street convention that `ch13::CleanPrice`
implements. QuantLib timings for options are for its instrument and engine
objects (each call sets the engine and asks for the NPV), which is how the
library is used; the plain `blackFormula` function is shown as well.

investm has no Python bindings of its C++ library, so the Python tables time
the investm Python package, which mirrors the C++ one.

## C++: investm against QuantLib

From `cpp/compare/compare_quantlib.cc`; raw results in
[`perf/compare-cpp.json`](perf/compare-cpp.json).

| Case | Ch. | investm | QuantLib | investm speed-up | Value (both) |
| --- | --- | --- | --- | --- | --- |
| BSM call (`blackFormula`) | 19 | 53 ns | 44 ns | 0.83x | 4.759422393 |
| BSM call (instrument + `AnalyticEuropeanEngine`) | 19 | 53 ns | 4.9 us | 94x | 4.759422393 |
| Price and five Greeks (`BlackCalculator`) | 19 | 53 ns | 226 ns | 4.3x | delta 0.779131 |
| Implied vol (QuantLib default accuracy 1e-6) | 19 | 0.97 us | 1.76 us | 1.8x | 0.2345129 / 0.2345124 |
| Implied vol (QuantLib at 1e-12) | 19 | 0.97 us | 2.84 us | 2.9x | 0.234512914 |
| CRR American put, 100 steps | 19 | 3.6 us | 72 us | 20x | 4.278059 / 4.278146 |
| CRR American put, 1,000 steps | 19 | 266 us | 5.67 ms | 21x | 4.283627 / 4.283636 |
| CRR European put, 1,000 steps | 19 | 5.6 us | 603 us | 109x | 4.074708 / 4.074719 |
| N(x), one point | 6 | 19 ns | 31 ns | 1.6x | |
| N^-1(p), one point | 6 | 64 ns | 15 ns | 0.23x | |
| Bond price at a yield | 5 | 139 ns | 7.3 us | 53x | 93.49603177 |
| Yield to maturity (QuantLib default 1e-10) | 5 | 2.1 us | 487 us | 230x | 0.09 |
| Yield to maturity (QuantLib at 1e-12) | 5 | 2.1 us | 567 us | 268x | 0.09 |
| Macaulay, modified duration and convexity | 14 | 0.99 us | 21.6 us | 22x | 6.954496, 6.655020, 58.3185 |
| Act/act accrued interest and clean price | 13 | 0.64 us | 15.2 us | 24x | 2.049724, 103.544528 |
| Bootstrap 30 annual-coupon bonds | 5 | 0.93 us | 2.07 ms | 2,200x | |
| Symmetric eigen-decomposition, n = 20 | 10 | 33 us | 107 us | 3.3x | |
| Symmetric eigen-decomposition, n = 100 | 10 | 1.75 ms | 11.0 ms | 6.3x | |
| Cholesky, n = 100 | 7 | 76 us | 98 us | 1.3x | |
| Inverse, n = 100 | 7 | 379 us | 44.9 ms | 118x | |

Accuracy, from the `error` counters:

| Case | investm | QuantLib | What is measured |
| --- | --- | --- | --- |
| N(x) on [-8, 8], 1,001 points | 9.0e-15 | 1.3e-7 | Largest relative error against a long double `erfc` |
| N^-1(p) on [1e-10, 0.5], 501 points | 1.3e-15 | 5.6e-9 | Largest absolute error in z against a Newton-polished long double root |
| Yield to maturity | -1.4e-17 | 2.4e-11 (1e-10), 5.2e-13 (1e-12) | Error against the 9% the price came from |
| Bootstrap | 2.2e-16 | 1.6e-15 | Largest discount-factor error against the true curve |
| Eigen, n = 100 | 5.2e-16 | 3.3e-16 | Largest residual of A v = lambda v, relative to lambda_max |
| Cholesky, n = 100 | 4.3e-18 | 4.3e-18 | Largest entry of L L' - A (entries of A are about 1e-3) |
| Inverse, n = 100 | 3.9e-15 | 3.1e-15 | Largest entry of A A^-1 - I |

Notes:

- **CRR trees:** QuantLib's tree gives a slightly different price at the same
  step count (it builds the lattice on its own time grid), but its error
  against the reference is the same to two digits (see the Python table).
- **Black-Scholes:** `BlackScholesPrice` is `BlackScholesGreeks(...).price`,
  so it pays for the shared density and both CDFs; the Greeks row shows the
  other side of that choice (six numbers for the price of one, 4.3x faster
  than `BlackCalculator`).
- **Bonds:** the values agree to all printed digits; the errors are in the
  Python table.

## Python: every library

From `scripts/compare/compare_python.py`; raw results in
[`perf/compare-python.json`](perf/compare-python.json). Times include each
library's Python call overhead, so QuantLib pays a few hundred nanoseconds
per call through SWIG.

### Black-Scholes price (Ch. 19)

Call, S = 42, K = 40, r = 10%, sigma = 20%, T = 0.5. Reference: the closed
form in mpmath.

| Library | Time | Error |
| --- | --- | --- |
| investm | 1.67 us | 1.7e-15 |
| QuantLib (`blackFormula`) | 256 ns | 2.6e-15 |
| vollib | 3.35 us | -1.8e-15 |

### Price and Greeks (Ch. 19)

Price, delta, gamma, vega, theta and rho of the same call in one call
(vollib needs six). Error: the largest relative error of the six.

| Library | Time | Error |
| --- | --- | --- |
| investm | 1.76 us | 3.6e-16 |
| QuantLib (`BlackCalculator`) | 5.62 us | 1.2e-15 |
| vollib (rescaled to per 1.00 and per year) | 15.1 us | 3.8e-16 |

### Implied volatility (Ch. 19)

Call at 1.875, S = 21, K = 20, r = 10%, T = 0.25. Reference: mpmath root of
the 50-digit price.

| Library | Time | Error |
| --- | --- | --- |
| investm (Brent, 1e-12) | 69.5 us | 9e-16 |
| QuantLib (default accuracy) | 1.49 us | -5.1e-7 |
| QuantLib (1e-12) | 2.23 us | -9.5e-13 |
| vollib (Let's Be Rational) | 15.1 us | -6.3e-16 |

### CRR trees (Ch. 19)

Put, S = K = 50, r = 10%, sigma = 40%, T = 5/12. American reference
4.284230 (mean of 20,000- and 20,001-step trees); European reference: the
Black-Scholes put.

| Library | American, 100 steps | American, 1,000 steps | Error at 1,000 | European, 1,000 steps | Error |
| --- | --- | --- | --- | --- | --- |
| investm | 484 us | 5.88 ms | -6.0e-4 | 36.6 us | -1.3e-3 |
| QuantLib | 68 us | 4.78 ms | -5.9e-4 | 830 us | -1.3e-3 |

### Normal distribution (Ch. 6)

Time for one point (x = -1.5, p = 0.05); errors over the grids of the C++
table, against mpmath.

| Library | N(x) | Rel. error | N^-1(p) | Abs. error in z |
| --- | --- | --- | --- | --- |
| investm | 119 ns | 9.0e-15 | 156 ns | 3.1e-15 |
| QuantLib | 459 ns | 1.3e-7 | 457 ns | 5.6e-9 |
| SciPy (`special.ndtr`, `ndtri`) | 120 ns | 9.5e-15 | 145 ns | 1.2e-15 |
| SciPy (`stats.norm.cdf`, `ppf`) | 36.2 us | 9.5e-15 | 49.8 us | 1.2e-15 |

`scipy.stats.norm` costs 300x the function it wraps for a scalar; it is
built for arrays.

### Bond price and yield (Ch. 5)

10-year 8% semiannual bond at a 9% yield, on a coupon date. Price
reference: exact rationals; yield reference: mpmath root.

| Library | Price | Error | Yield | Error |
| --- | --- | --- | --- | --- |
| investm | 1.90 us | -3.7e-15 | 37.1 us | 8e-18 |
| QuantLib | 6.55 us | 2.2e-13 | 80.6 us | 3.7e-16 |
| numpy-financial (`pv`; `rate`, default tol) | 8.63 us | 1.1e-14 | 89.8 us | 1.2e-12 |
| numpy-financial (`npv`; `rate`, tol 1e-12) | 4.88 us | 8.2e-14 | 103 us | 8e-18 |
| SciPy (`brentq` on numpy-financial's `pv`) | | | 130 us | 3.6e-17 |

### Duration and convexity (Ch. 14)

Same bond. Error: the largest relative error of Macaulay duration, modified
duration and convexity, against exact rationals.

| Library | Time | Error |
| --- | --- | --- |
| investm | 9.96 us | 5.2e-16 |
| QuantLib | 22.2 us | 2.8e-16 |

### Accrued interest and clean price, act/act (Ch. 13)

7% Treasury maturing 15 November 2036, settled 1 March 2027 (106 of 181
days into the coupon period), yield 6.5%. Accrued interest 2.049724, clean
price 103.544528.

| Library | Time | Error |
| --- | --- | --- |
| investm | 64.8 us | 3.5e-14 |
| QuantLib (`ActualActual(ISMA)`) | 8.78 us | 1.2e-13 |

### Bootstrapping (Ch. 5)

30 annual-coupon bonds (coupons 4.05% to 5.5%) priced off the curve
s(t) = 3% + 2% (1 - e^(-t/5)). Error: the largest discount-factor error.

| Library | Time | Error |
| --- | --- | --- |
| investm | 14.0 us | 2.2e-16 |
| QuantLib (`PiecewiseLogLinearDiscount`) | 1.30 ms | 1.6e-15 |

### Internal rate of return (Ch. 5, 24)

An 11-period stream with three sign changes, as in a dollar-weighted return.
Reference: mpmath root, 12.53528785%.

| Library | Time | Error |
| --- | --- | --- |
| investm (Brent) | 20.2 us | -5e-19 |
| numpy-financial (`irr`, companion-matrix eigenvalues) | 55.7 us | 1.9e-15 |
| SciPy (`brentq` on numpy-financial's `npv`) | 78.2 us | 1.8e-14 |

### Portfolio optimization (Ch. 7, 8)

Expected returns uniform on [4%, 16%], covariance from a random factor
matrix, r_f = 3%, risk tolerance 0.3. Error: the shortfall in Sharpe ratio
(or objective) against the best any library reached; for the unconstrained
tangency, against the closed form.

| Problem | investm | PyPortfolioOpt | SciPy SLSQP |
| --- | --- | --- | --- |
| Tangency, 20 assets, short sales allowed | 57 us, 0 | 6.11 ms, -7e-16 | |
| Max Sharpe, 20 assets, no short sales | 3.76 ms, 2e-16 | 6.52 ms, 0 | 3.38 ms, 0 |
| Max Sharpe, 100 assets, no short sales | 10.9 ms, 0 | 23.7 ms, 9e-16 | 253 ms, 2.2e-10 |
| Max tau mu'w - w'Sw, 20 assets, no short sales | 0.93 ms, 0 | 6.37 ms, 7e-18 | 3.25 ms, 6e-17 |
| Max tau mu'w - w'Sw, 100 assets, no short sales | 2.28 ms, 0 | 9.57 ms, 2e-17 | 96.9 ms, 2e-15 |

investm and PyPortfolioOpt solve the max-Sharpe problem the same way (one
QP in z = w / k with (mu - r_f)'z = 1); investm uses its own accelerated
projected gradient, PyPortfolioOpt builds a cvxpy problem and calls
Clarabel. SLSQP works on the Sharpe ratio directly with an analytic
gradient and keeps a dense quasi-Newton matrix, so it slows down quickly
with n.

### CAPM-implied returns and style analysis (Ch. 9, 24)

| Case | investm | Other | Error (investm / other) |
| --- | --- | --- | --- |
| Implied returns, 20 assets (PyPortfolioOpt `market_implied_prior_returns`) | 4.13 us | 262 us | 3e-17 / 2e-17 |
| Style analysis, 120 x 8 (cvxpy, Clarabel) | 0.87 ms | 5.75 ms | 0 / 3e-21 |
| Style analysis, 120 x 8 (SciPy SLSQP) | 0.87 ms | 0.76 ms | 0 / 4e-18 |

PyPortfolioOpt's prior is delta S w + r_f; with delta = market premium /
var(market) it is the book's r_f + beta (E[r_M] - r_f). The style-analysis
error is the excess tracking variance (about 2.7e-5) over the best fit.

### Regressions and tests of efficiency (Ch. 4, 7, 24)

References: exact rational least squares for the regressions, mpmath for
the tests.

| Case | Library | Time | Error |
| --- | --- | --- | --- |
| Market model, 60 months | investm | 74.9 us | 6.5e-17 |
| | statsmodels (`OLS`, all statistics) | 124 us | 6.0e-16 |
| | SciPy (`linregress`) | 406 us | 2.9e-16 |
| | NumPy (`polyfit`) | 24.9 us | 8.2e-16 |
| Treynor-Mazuy, 120 months | investm | 104 us | 5.2e-16 |
| | statsmodels | 173 us | 4.7e-16 |
| Runs test z, 500 returns | investm | 96 us | 5e-16 |
| | statsmodels (`runstest_1samp`) | 98 us | 5e-16 |
| Variance ratio VR(4), 1,000 returns | investm | 873 us | -4e-16 |
| | arch (`debiased=False`) | 366 us | -3.1e-3 (definition) |
| | arch (default, debiased) | 385 us | 3.1e-3 (definition) |
| Lag-1 autocorrelation, 500 returns | investm | 292 us | -1e-17 |
| | statsmodels (`acf`) | 62 us | -3e-17 |
| | pandas (`autocorr`) | 162 us | 2.4e-4 (definition) |

The timing t-statistics agree to all printed digits (5.033555), as do the
market-model betas.

## Follow-ups

- **Python implied volatility:** every Brent step builds a new
  `OptionSpec` with `dataclasses.replace` and computes all six Greeks.
  Pricing with a plain function of sigma should bring it close to vollib.
- **Python act/act pricing:** `surrounding_coupons` steps back from maturity
  one coupon at a time with `add_months`; jumping straight to the right
  period would make it O(1).
- **Python variance ratio and autocorrelation:** both loop in pure Python
  (`math.fsum` over generators). NumPy versions would match arch and
  statsmodels.
- **Python CRR tree at 100 steps:** the per-step NumPy overhead dominates
  small trees; a pure-Python loop below a few hundred steps would be faster.
- **C++ `NormInv`:** QuantLib shows the Acklam step alone takes 15 ns; the
  Newton refinement costs the other 50 ns. A Halley step on the tail branch
  only, or Wichura's AS241 (as the Python version uses), could keep 1e-15
  for less.

## Reproducing

```bash
sudo apt-get install -y libquantlib0-dev libbenchmark-dev
pip install -r scripts/compare/requirements.txt
scripts/compare.sh            # writes docs/perf/compare-cpp.json and compare-python.json
```

or step by step:

```bash
cmake -S . -B build/compare -DCMAKE_BUILD_TYPE=Release \
    -DINVESTM_BUILD_TESTS=OFF -DINVESTM_BUILD_EXAMPLES=OFF -DINVESTM_BUILD_COMPARE=ON
cmake --build build/compare --target investm_compare_quantlib
build/compare/cpp/compare/investm_compare_quantlib --benchmark_repetitions=5 \
    --benchmark_report_aggregates_only=true \
    --benchmark_out=docs/perf/compare-cpp.json --benchmark_out_format=json

OMP_NUM_THREADS=1 OPENBLAS_NUM_THREADS=1 \
    python3 scripts/compare/compare_python.py --out docs/perf/compare-python.json
```

On Windows, the Docker image built from `docker/Dockerfile` has everything:

```powershell
docker run --rm -v C:\develop:/develop -w /develop/investm investm-dev scripts/compare.sh
```
