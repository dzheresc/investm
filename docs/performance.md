# Performance testing and profiling

This page records one round of measure, profile, fix, and re-measure on the
whole companion. The tools live in the repository, so the round can be
repeated after any change.

| Tool | What it does |
|---|---|
| `build/cpp/bench/investm_bench` | Times each case (median and minimum per call over repetitions). Options: `--filter=substr`, `--min_time=s`, `--repetitions=n`, `--csv=path`, `--quick` (one call per case; also a ctest smoke test). |
| `scripts/profile.sh [tool...]` | Runs the valgrind suite (memcheck, callgrind, cachegrind, massif, DHAT) on the benchmark driver; memcheck also covers every `chNN_examples` binary. Reports go to `build/profile/reports`. `FILTER=ch08 scripts/profile.sh callgrind` narrows the run. |
| `scripts/bench_python.py [dir]` | Times the Python versions of the same hot paths. Passing another checkout's `python/` directory compares two versions. |

Helgrind and DRD are not part of the suite because the code is single threaded.

## Setup

Numbers come from a 4-core Intel Xeon at 2.1 GHz, GCC 13.3, `RelWithDebInfo`
(`-O2 -g`), Ubuntu 24.04. The machine is a shared VM. Kernels that did not
change vary by up to ±30% between runs, so read only large ratios as real.
The raw data is in [perf/before.csv](perf/before.csv) and
[perf/after.csv](perf/after.csv).

## What profiling found

1. **The QP recomputed a full eigen-decomposition on every call.**
   `OptimalNoShortSales` (Ch. 7) used `EigenSymmetric(cov).values.front()` to
   get the FISTA step size. Only the largest eigenvalue is needed, and power
   iteration finds it in O(n²) per step.
2. **The no-short-sales tangency portfolio (Ch. 8) ran about 40 QPs.** It used
   a golden-section search on risk tolerance. Callgrind on
   `tangency_no_short/100` attributed 60.5% of instructions to
   `EigenSymmetric` and 32% to matrix-vector products. Each QP started cold
   and paid for its own eigen-decomposition.
3. **The eigen solver was cyclic Jacobi.** Each sweep costs O(n³) with
   column-strided access, and it needs several sweeps. This was 137 ms at
   n = 200 and dominated principal components (Ch. 10).
4. **FISTA without restart oscillates.** On these strongly convex problems,
   plain Nesterov momentum overshoots and runs many iterations. The loop also
   allocated three vectors per iteration: `cov * y`, the projection result,
   and the sort buffer. DHAT counted 210,650 heap blocks for one tangency
   solve at n = 100.
5. **The American binomial tree called `std::pow` twice per node** to rebuild
   the stock price for the early-exercise check, which is O(n²) `pow` calls.

## Fixes

| Area | Change |
|---|---|
| `core/matrix` | `EigenSymmetric` uses Householder tridiagonalization plus implicit QL (tred2/tql2). It adds `LargestEigenvalue`: power iteration padded by its residual norm, so the result is a safe upper bound for a step size. It also adds `MultiplyInto` for allocation-free matrix-vector products with four accumulators. |
| `ch07` | `AffineSimplexProjector` gives an exact projection onto {z ≥ 0, a'z = 1} for any signs in `a`, by sorting the breakpoints vᵢ/aᵢ. `SolveAffineSimplexQp` runs FISTA with gradient-based adaptive restart (O'Donoghue and Candès, 2015) and a warm start, with no allocation inside the loop. `EfficientSetNoShortSales` computes the step once and warm-starts each risk tolerance from the previous one. |
| `ch08` | The tangency portfolio is one QP: minimize z'Σz subject to (μ − r_f)'z = 1 and z ≥ 0, then set w = z / 1'z. This works because the Sharpe ratio does not depend on scale. The golden-section search remains only for the case where no asset beats r_f, and it now warm-starts too. |
| `ch19` | Node (i, j) of the tree has price S·u^(2j−i), so each step back multiplies by u. Only the n + 1 terminal prices use `exp`. |
| `ch24` | Style analysis uses the shared QP solver. |
| Python | Mirrors the same algorithms: direct tangency QP, restart, warm start, general projection, and multiplicative binomial prices. |

New tests cover all of these. The eigen solver is checked on random SPD
matrices (A V = V Λ, orthonormal V, sorted values) and on diagonal, zero,
repeated-eigenvalue, 1×1 and empty matrices. The projection is checked
against KKT conditions with mixed-sign `a`. The tangency portfolio is checked
against 2,000 random long-only portfolios, and against the all-below-r_f
fallback. Warm and cold starts must agree.

## Results (C++)

| Case | Before | After | Speed-up |
|---|---:|---:|---:|
| `ch08/tangency_no_short/100` | 877 ms | 0.92 ms | **959×** |
| `ch08/tangency_no_short/20` | 14.5 ms | 44.7 µs | **325×** |
| `ch07/efficient_set_no_short/100x8` | 125 ms | 4.18 ms | **30×** |
| `ch07/optimal_no_short_sales/100` | 15.9 ms | 618 µs | **26×** |
| `ch07/efficient_set_no_short/20x8` | 1.55 ms | 139 µs | 11× |
| `ch07/optimal_no_short_sales/20` | 181 µs | 19.2 µs | 9.4× |
| `ch19/binomial_american/1000` | 13.0 ms | 560 µs | **23×** |
| `ch19/binomial_american/100` | 134 µs | 9.8 µs | 14× |
| `core/eigen_symmetric/200` | 137 ms | 10.0 ms | **14×** |
| `core/eigen_symmetric/50` | 1.28 ms | 280 µs | 4.6× |
| `ch10/principal_components/250x100` | 10.4 ms | 2.09 ms | 5.0× |
| `ch24/style_analysis/120x8` | 33.8 µs | 14.5 µs | 2.3× |
| other kernels (matmul, LU, Cholesky, YTM, Black-Scholes, ...) | | | unchanged, within noise |

The same cases under valgrind, counting only the case and excluding the
fixed setup cost of the benchmark driver (about 160 M instructions and 813
blocks):

| Case | Instructions before → after | Heap blocks before → after |
|---|---|---|
| `tangency_no_short/100` | 13.2 G → 12.5 M | 209,837 → 14 |
| `efficient_set_no_short/100x8` | 2.10 G → 62 M | 24,599 → 71 |
| `binomial_american/1000` | 155 M → 13 M | 2 → 3 |

## Results (Python, `scripts/bench_python.py`)

| Case | Before | After | Speed-up |
|---|---:|---:|---:|
| `ch08/tangency_no_short/100` | 932 ms | 10 ms | 92× |
| `ch08/tangency_no_short/20` | 1274 ms | 10 ms | 125× |
| `ch07/efficient_set_no_short/100x8` | 83 ms | 16 ms | 5× |
| `ch07/optimal_no_short/100` | 5.2 ms | 1.9 ms | 2.8× |
| `ch19/binomial_american/1000` | 14.6 ms | 5.4 ms | 2.7× |
| `ch24/style_analysis/120x8` | 1.13 ms | 0.82 ms | 1.4× |

In Python the per-iteration interpreter overhead is larger than the
arithmetic, so the gains come from doing fewer iterations, not from cheaper
ones.

## Valgrind summary after the fixes

- **memcheck**: 0 errors and 0 leaks across all 25 example binaries and the
  benchmark driver. CI runs the examples under memcheck on every push.
- **callgrind**: the hot spots are now the textbook kernels.
  `EigenSymmetric` is 32% of the benchmark run (at n = 200 it is
  intrinsically O(n³)), followed by `CovarianceMatrix` (17%), matrix multiply
  (9%), LU (14%) and `MultiplyInto` (7.5%). No finance-level function
  appears above 2%.
- **cachegrind**: D1 miss rate 3.2%, LL miss rate 0.0%, branch mispredict
  rate 1.4%. The working sets fit in cache.
- **massif**: peak heap 4.4 MB, set by the 1000×100 covariance input and
  the 250×100 PCA panel built for the benchmark.
- **DHAT**: 1,650 blocks and 14.8 MB in total for the whole quick run, with
  0 bytes live at exit.

## Left as is

- Dense matmul, LU and covariance are not cache blocked and do not
  auto-vectorize at `-O2`. At `-O2` GCC does not add runtime alias checks, and
  the output may alias the inputs. A `-O3 -march=native` build or a BLAS would
  speed them up, but portfolio problems in the book have tens of assets, so
  this code favours readability and has no dependencies.
