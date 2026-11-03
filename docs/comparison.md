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

The C++ tables ran in the cloud container and the Python tables on a desktop
in Docker, with the processes pinned (see [Setup](#setup)). Close Python
cases ranked differently on the two machines, so treat a gap of a few
percent as a tie. A second round closed the speed gaps the first one found;
see [Gaps closed](#gaps-closed). In the C++ tables, rows for code that
changed are from the second run; the other rows are from the first, and the
fresh JSON agrees with them within the container's noise.

## Summary

- **Every investm result agrees with its reference to 4e-14 or better,**
  in both languages (the binomial trees apart, whose discretization error
  matches QuantLib's). The comparison found no bug in investm.
- **C++ against QuantLib: investm is faster, or level, in every case.** Bond
  price, yield, duration and accrued interest are 20-270x faster: investm
  works on a vector of (time, amount) pairs, QuantLib on its general leg of
  dated coupon objects and a day counter. The American CRR
  tree is 21x faster at 1,000 steps (QuantLib's generic lattice framework
  against investm's single array), and the European tree is 109x faster
  because investm sums the terminal payoffs in O(n) instead of rolling back.
  The bond bootstrap is 2,200x faster: investm solves one linear equation
  per maturity, QuantLib runs an iterative piecewise-curve bootstrap.
- **The two small functions that were slower are now level.** The first
  round found QuantLib's `blackFormula` 1.2x faster than `BlackScholesPrice`
  and `InverseCumulativeNormal` 4.3x faster than `ch06::NormInv`. Now
  `BlackScholesPrice` takes 47 ns against 44 ns (within noise) and
  `NormInv` 15 ns against 13 ns, with an error of 3e-15 against QuantLib's
  5.6e-9.
- **Python: investm's NumPy code holds its own.** The no-short-sales
  optimizers are the fastest of PyPortfolioOpt (cvxpy), SciPy SLSQP and
  investm at 100 assets (4.5x faster than PyPortfolioOpt for the max-Sharpe
  portfolio, 12x for the quadratic-utility one, 19-43x faster than SLSQP),
  and every optimizer reaches the same optimum to 6e-15, except SLSQP on the
  100-asset max-Sharpe problem (4.0e-10 short).
- **Python gaps that remain:** investm is behind another library on implied
  volatility (10.8 us; vollib 18 us, QuantLib's C++ through SWIG 1.2 us),
  N(x) (190 ns; SciPy 112 ns), the variance ratio (365 us; arch 271 us, with
  a different definition), the autocorrelation (106 us; statsmodels 53 us),
  the runs test (90 us; statsmodels 83 us), the act/act clean price (24 us;
  QuantLib 6.7 us) and the 100-step CRR tree (346 us; QuantLib 49 us). The
  second round had already cut implied volatility, the variance ratio, the
  autocorrelation and the clean price by 3-10x; see
  [Gaps closed](#gaps-closed).

Accuracy findings in the other libraries:

1. **QuantLib's `CumulativeNormalDistribution` has a relative error of
   1.3e-7 in the far left tail** (x near -8; the absolute error is 1.2e-15).
   For p-values and tail probabilities that is 7 significant digits. investm
   and SciPy use `erfc` and are good to 9e-15 relative.
2. **QuantLib's `InverseCumulativeNormal` is good to 5.6e-9 in z** (Acklam's
   approximation). investm (Wichura's AS241 in both languages) and SciPy
   are good to 3e-15.
3. **Default tolerances cost digits.** QuantLib's implied volatility at its
   default accuracy (1e-6) is off by 5.1e-7; numpy-financial's `rate` at its
   default tolerance (1e-6) gives a yield off by 1.2e-12. Both are exact to
   rounding when asked for 1e-12.
4. **QuantLib 1.33's `BondFunctions::yield` takes 0.49 ms in C++, while the
   1.44 wheel takes 81 us from Python** in the same container for the same
   bond, so the difference is the version, not the binding. It was not investigated
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
| Host (C++) | Cloud container, 4 cores at 2.1 GHz, Ubuntu 24.04 (noisy; see [Performance](performance.md)) |
| Host (Python) | Desktop, Core Ultra 9 185H, the `docker/Dockerfile` image (Ubuntu 24.04) on Docker Desktop and WSL2, pinned to CPUs 0, 2, 4 and 6 |
| C++ | GCC 13.3, `Release` (-O3), Google Benchmark 1.8.3, median of 5 repetitions |
| QuantLib (C++) | 1.33, Ubuntu 24.04 package `libquantlib0-dev` |
| Python | 3.13, NumPy 2.5.4, best of 5 `timeit` runs |
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
| BSM call (`blackFormula`) | 19 | 47 ns | 44 ns | 0.94x | 4.759422393 |
| BSM call (instrument + `AnalyticEuropeanEngine`) | 19 | 47 ns | 4.9 us | 105x | 4.759422393 |
| Price and five Greeks (`BlackCalculator`) | 19 | 53 ns | 226 ns | 4.3x | delta 0.779131 |
| Implied vol (QuantLib default accuracy 1e-6) | 19 | 0.45 us | 1.71 us | 3.8x | 0.2345129 / 0.2345124 |
| Implied vol (QuantLib at 1e-12) | 19 | 0.45 us | 2.94 us | 6.5x | 0.234512914 |
| CRR American put, 100 steps | 19 | 3.6 us | 72 us | 20x | 4.278059 / 4.278146 |
| CRR American put, 1,000 steps | 19 | 266 us | 5.67 ms | 21x | 4.283627 / 4.283636 |
| CRR European put, 1,000 steps | 19 | 5.6 us | 603 us | 109x | 4.074708 / 4.074719 |
| N(x), one point | 6 | 19 ns | 31 ns | 1.6x | |
| N^-1(p), one point | 6 | 15 ns | 13 ns | 0.86x | |
| Bond price at a yield | 5 | 139 ns | 7.3 us | 53x | 93.49603177 |
| Yield to maturity (QuantLib default 1e-10) | 5 | 2.1 us | 487 us | 230x | 0.09 |
| Yield to maturity (QuantLib at 1e-12) | 5 | 2.1 us | 567 us | 268x | 0.09 |
| Macaulay, modified duration and convexity | 14 | 0.99 us | 21.6 us | 22x | 6.954496, 6.655020, 58.3185 |
| Act/act accrued interest and clean price | 13 | 0.48 us | 15.4 us | 32x | 2.049724, 103.544528 |
| Bootstrap 30 annual-coupon bonds | 5 | 0.93 us | 2.07 ms | 2,200x | |
| Symmetric eigen-decomposition, n = 20 | 10 | 33 us | 107 us | 3.3x | |
| Symmetric eigen-decomposition, n = 100 | 10 | 1.75 ms | 11.0 ms | 6.3x | |
| Cholesky, n = 100 | 7 | 76 us | 98 us | 1.3x | |
| Inverse, n = 100 | 7 | 379 us | 44.9 ms | 118x | |

Accuracy, from the `error` counters:

| Case | investm | QuantLib | What is measured |
| --- | --- | --- | --- |
| N(x) on [-8, 8], 1,001 points | 9.0e-15 | 1.3e-7 | Largest relative error against a long double `erfc` |
| N^-1(p) on [1e-10, 0.5], 501 points | 3.1e-15 | 5.6e-9 | Largest absolute error in z against a Newton-polished long double root |
| Yield to maturity | -1.4e-17 | 2.4e-11 (1e-10), 5.2e-13 (1e-12) | Error against the 9% the price came from |
| Bootstrap | 2.2e-16 | 1.6e-15 | Largest discount-factor error against the true curve |
| Eigen, n = 100 | 5.2e-16 | 3.3e-16 | Largest residual of A v = lambda v, relative to lambda_max |
| Cholesky, n = 100 | 4.3e-18 | 4.3e-18 | Largest entry of L L' - A (entries of A are about 1e-3) |
| Inverse, n = 100 | 3.9e-15 | 3.1e-15 | Largest entry of A A^-1 - I |

Notes:

- **CRR trees:** QuantLib's tree gives a slightly different price at the same
  step count (it builds the lattice on its own time grid), but its error
  against the reference is the same to two digits (see the Python table).
- **Black-Scholes:** `BlackScholesGreeks` returns all six numbers for
  the price of one, 4.1x faster than `BlackCalculator`.
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
| investm | 1.21 us | -5.4e-15 |
| QuantLib (`blackFormula`) | 309 ns | 2.6e-15 |
| vollib | 4.21 us | -1.8e-15 |

### Price and Greeks (Ch. 19)

Price, delta, gamma, vega, theta and rho of the same call in one call
(vollib needs six). Error: the largest relative error of the six.

| Library | Time | Error |
| --- | --- | --- |
| investm | 2.58 us | 3.6e-16 |
| QuantLib (`BlackCalculator`) | 4.76 us | 1.2e-15 |
| vollib (rescaled to per 1.00 and per year) | 14.5 us | 3.8e-16 |

### Implied volatility (Ch. 19)

Call at 1.875, S = 21, K = 20, r = 10%, T = 0.25. Reference: mpmath root of
the 50-digit price.

| Library | Time | Error |
| --- | --- | --- |
| investm (Newton in a bracket) | 10.8 us | 8.1e-16 |
| QuantLib (default accuracy) | 1.17 us | -5.1e-7 |
| QuantLib (1e-12) | 1.64 us | -9.5e-13 |
| vollib (Let's Be Rational) | 18.1 us | -6.3e-16 |

### CRR trees (Ch. 19)

Put, S = K = 50, r = 10%, sigma = 40%, T = 5/12. American reference
4.284230 (mean of 20,000- and 20,001-step trees); European reference: the
Black-Scholes put.

| Library | American, 100 steps | American, 1,000 steps | Error at 1,000 | European, 1,000 steps | Error |
| --- | --- | --- | --- | --- | --- |
| investm | 346 us | 4.15 ms | -6.0e-4 | 35.1 us | -1.3e-3 |
| QuantLib | 49.4 us | 3.41 ms | -5.9e-4 | 521 us | -1.3e-3 |

### Normal distribution (Ch. 6)

Time for one point (x = -1.5, p = 0.05); errors over the grids of the C++
table, against mpmath.

| Library | N(x) | Rel. error | N^-1(p) | Abs. error in z |
| --- | --- | --- | --- | --- |
| investm | 190 ns | 9.0e-15 | 216 ns | 3.1e-15 |
| QuantLib | 433 ns | 1.3e-7 | 458 ns | 5.6e-9 |
| SciPy (`special.ndtr`, `ndtri`) | 112 ns | 9.5e-15 | 126 ns | 1.2e-15 |
| SciPy (`stats.norm.cdf`, `ppf`) | 29.2 us | 9.5e-15 | 43.1 us | 1.2e-15 |

`scipy.stats.norm` costs 300x the function it wraps for a scalar; it is
built for arrays.

### Bond price and yield (Ch. 5)

10-year 8% semiannual bond at a 9% yield, on a coupon date. Price
reference: exact rationals; yield reference: mpmath root.

| Library | Price | Error | Yield | Error |
| --- | --- | --- | --- | --- |
| investm | 2.77 us | -3.7e-15 | 48.6 us | 8e-18 |
| QuantLib | 4.74 us | 2.2e-13 | 52.3 us | 3.7e-16 |
| numpy-financial (`pv`; `rate`, default tol) | 6.61 us | -3.7e-15 | 60.3 us | 1.2e-12 |
| numpy-financial (`npv`; `rate`, tol 1e-12) | 4.17 us | 8.2e-14 | 70 us | -6e-18 |
| SciPy (`brentq` on numpy-financial's `pv`) | | | 89 us | 3.6e-17 |

### Duration and convexity (Ch. 14)

Same bond. Error: the largest relative error of Macaulay duration, modified
duration and convexity, against exact rationals.

| Library | Time | Error |
| --- | --- | --- |
| investm | 14.2 us | 5.2e-16 |
| QuantLib | 14.7 us | 2.8e-16 |

### Accrued interest and clean price, act/act (Ch. 13)

7% Treasury maturing 15 November 2036, settled 1 March 2027 (106 of 181
days into the coupon period), yield 6.5%. Accrued interest 2.049724, clean
price 103.544528.

| Library | Time | Error |
| --- | --- | --- |
| investm | 24.3 us | 3.5e-14 |
| QuantLib (`ActualActual(ISMA)`) | 6.74 us | 1.2e-13 |

### Bootstrapping (Ch. 5)

30 annual-coupon bonds (coupons 4.05% to 5.5%) priced off the curve
s(t) = 3% + 2% (1 - e^(-t/5)). Error: the largest discount-factor error.

| Library | Time | Error |
| --- | --- | --- |
| investm | 18.8 us | 2.2e-16 |
| QuantLib (`PiecewiseLogLinearDiscount`) | 780 us | 1.6e-15 |

### Internal rate of return (Ch. 5, 24)

An 11-period stream with three sign changes, as in a dollar-weighted return.
Reference: mpmath root, 12.53528785%.

| Library | Time | Error |
| --- | --- | --- |
| investm (Brent) | 26.4 us | -5e-19 |
| numpy-financial (`irr`, companion-matrix eigenvalues) | 41.2 us | 1.9e-15 |
| SciPy (`brentq` on numpy-financial's `npv`) | 59.5 us | 1.8e-14 |

### Portfolio optimization (Ch. 7, 8)

Expected returns uniform on [4%, 16%], covariance from a random factor
matrix, r_f = 3%, risk tolerance 0.3. Error: the shortfall in Sharpe ratio
(or objective) against the best any library reached; for the unconstrained
tangency, against the closed form.

| Problem | investm | PyPortfolioOpt | SciPy SLSQP |
| --- | --- | --- | --- |
| Tangency, 20 assets, short sales allowed | 41.4 us, 0 | 12.6 ms, -2e-16 | |
| Max Sharpe, 20 assets, no short sales | 2.92 ms, 5e-15 | 13 ms, 6e-15 | 2.45 ms, 0 |
| Max Sharpe, 100 assets, no short sales | 8.56 ms, 4e-15 | 38.7 ms, 0 | 165 ms, 4.0e-10 |
| Max tau mu'w - w'Sw, 20 assets, no short sales | 0.76 ms, 7e-18 | 12.9 ms, 0 | 2.48 ms, 6e-17 |
| Max tau mu'w - w'Sw, 100 assets, no short sales | 1.50 ms, 0 | 17.9 ms, 5e-17 | 64.3 ms, 2e-15 |

investm and PyPortfolioOpt solve the max-Sharpe problem the same way (one
QP in z = w / k with (mu - r_f)'z = 1); investm uses its own accelerated
projected gradient, PyPortfolioOpt builds a cvxpy problem and calls
Clarabel. SLSQP works on the Sharpe ratio directly with an analytic
gradient and keeps a dense quasi-Newton matrix, so it slows down quickly
with n.

### CAPM-implied returns and style analysis (Ch. 9, 24)

| Case | investm | Other | Error (investm / other) |
| --- | --- | --- | --- |
| Implied returns, 20 assets (PyPortfolioOpt `market_implied_prior_returns`) | 3.24 us | 227 us | 1e-17 / 2e-17 |
| Style analysis, 120 x 8 (cvxpy, Clarabel) | 0.74 ms | 4.98 ms | 0 / 3e-21 |
| Style analysis, 120 x 8 (SciPy SLSQP) | 0.74 ms | 0.59 ms | 0 / 4e-18 |

PyPortfolioOpt's prior is delta S w + r_f; with delta = market premium /
var(market) it is the book's r_f + beta (E[r_M] - r_f). The style-analysis
error is the excess tracking variance (about 2.7e-5) over the best fit.

### Regressions and tests of efficiency (Ch. 4, 7, 24)

References: exact rational least squares for the regressions, mpmath for
the tests.

| Case | Library | Time | Error |
| --- | --- | --- | --- |
| Market model, 60 months | investm | 54.8 us | 6.5e-17 |
| | statsmodels (`OLS`, all statistics) | 115 us | 6.0e-16 |
| | SciPy (`linregress`) | 337 us | 2.9e-16 |
| | NumPy (`polyfit`) | 23.2 us | 6.0e-16 |
| Treynor-Mazuy, 120 months | investm | 93.5 us | 5.2e-16 |
| | statsmodels | 156 us | 4.7e-16 |
| Runs test z, 500 returns | investm | 90 us | 5e-16 |
| | statsmodels (`runstest_1samp`) | 83 us | 5e-16 |
| Variance ratio VR(4), 1,000 returns | investm | 365 us | 6e-17 |
| | arch (`debiased=False`) | 274 us | -3.1e-3 (definition) |
| | arch (default, debiased) | 271 us | 3.1e-3 (definition) |
| Lag-1 autocorrelation, 500 returns | investm | 106 us | -1e-17 |
| | statsmodels (`acf`) | 53 us | -3e-17 |
| | pandas (`autocorr`) | 129 us | 2.4e-4 (definition) |

The timing t-statistics agree to all printed digits (5.033555), as do the
market-model betas.

## Gaps closed

The first round listed places where investm was slower than another library
on the same inputs. A second round, the same pass the ofod11 companion made
after its comparison, changed the code behind most of them. Results that
were exact before are still exact, and each change has a new test. The times
in this table are all from the cloud container, so each before and after
pair comes from one machine; the Python tables above have the desktop times.

| Case | Before | After | Other library | Change |
| --- | --- | --- | --- | --- |
| C++ `NormInv` | 64 ns | 15 ns | QuantLib 13 ns (error 5.6e-9) | Wichura's AS241, accurate to about 1e-16 without refinement, replaces Acklam plus a Newton step against `erfc`. The Python version already used AS241. Test: round trip N(z) = p from 1e-300 to 0.5. |
| C++ `BlackScholesPrice` | 53 ns | 47 ns | QuantLib 44 ns | Computes the price alone instead of all six Greeks. |
| C++ `ImpliedVolatility` | 0.97 us | 0.45 us | QuantLib 1.7 us at 1e-6 | Newton on vega, started at the inflection point sqrt(2 &#124;ln(S e^-qT / K e^-rT)&#124; / T), inside a bracket that narrows as it goes, with Brent as the fallback. Test: round trip over a grid of up to 700 options (both types, K = 40 to 250, T = 0.02 to 30 years, sigma = 3% to 150%, with and without dividends). |
| Python `implied_volatility` | 69.5 us | 6.8 us | vollib 15 us, QuantLib 1.5 us | The same search, on a price function of sigma alone instead of `black_scholes_price` with a new `OptionSpec` per step. Same test. |
| Python act/act clean price | 65 us | 22 us | QuantLib 8.8 us | `surrounding_coupons` jumps to the coupon period from the months to maturity instead of stepping back one coupon at a time (also in C++: 0.64 to 0.48 us). Test: agrees with the old walk on every third day of several years, for month-end maturities and every frequency. |
| Python `variance_ratio` | 873 us | 251 us | arch 407 us | One pass over plain floats; `stats.variance` and `covariance` take `math.fsum` of `map(operator.mul, ...)` instead of generators. |
| Python `autocorrelation` | 292 us | 83 us | statsmodels 64 us | Converts NumPy input to floats once, then the same `fsum` of products. |

Left as they are:

- **Python CRR tree at 100 steps** (346 us against QuantLib's 49 us
  through SWIG): each step is a few NumPy calls on short arrays, and their
  overhead dominates small trees. A pure-Python loop is slower still; the
  fast path is the C++ tree (3.9 us), which investm does not bind to
  Python.
- **Python act/act clean price** is still 3.6x QuantLib's: what remains is
  `datetime` arithmetic in `add_months` and the day counts.
- `core/stats` and Chapter 4 stay pure Python (no NumPy), as the layout in
  the README describes for the early chapters.

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

The Python tables here came from `--cpuset-cpus=0,2,4,6` after `--rm` and
`scripts/compare.sh python`.
