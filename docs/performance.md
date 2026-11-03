# Performance testing and profiling

This page records two rounds of measure, profile, fix, and re-measure on the
whole companion. [Round 2](#round-2-callgrind-and-cachegrind-across-all-chapters)
is the newer one. The tools live in the repository, so the round can be
repeated after any change.
[Comparison results](comparison.md) times the same kinds of kernels against
QuantLib and other open-source libraries; its
[Gaps closed](comparison.md#gaps-closed) section is the third round of
fixes, for the cases where another library was faster.

| Tool | What it does |
|---|---|
| `build/cpp/bench/investm_bench` | Times each case (median and minimum per call over repetitions). Options: `--filter=substr`, `--min_time=s`, `--repetitions=n`, `--csv=path`, `--quick` (one call per case; also a ctest smoke test). |
| `scripts/profile.sh [tool...]` | Runs the valgrind suite (memcheck, callgrind, cachegrind, massif, DHAT) on the benchmark driver; memcheck also covers every `chNN_examples` binary. `chapters` runs callgrind over every example and test binary; `cases` profiles each benchmark case alone, with cache and branch simulation (`BASELINE=dir` compares against an earlier run). Reports go to `build/profile/reports`. `FILTER=ch08 scripts/profile.sh callgrind` narrows the run. |
| `scripts/callgrind_cases.py dir [baseline]` | Tabulates the per-case dumps that `investm_bench --quick` writes under callgrind. |
| `scripts/docker_profile.ps1 [tool...]` | Windows: builds `docker/Dockerfile` and runs `profile.sh` in it with `C:\develop` mounted. |
| `scripts/bench_python.py [dir]` | Times the Python versions of the same hot paths. Passing another checkout's `python/` directory compares two versions. |

Helgrind and DRD are not part of the suite because the code is single threaded.

## Round 1: tangency portfolio, QP and eigen solver

### Setup

Numbers come from a 4-core Intel Xeon at 2.1 GHz, GCC 13.3, `RelWithDebInfo`
(`-O2 -g`), Ubuntu 24.04. The machine is a shared VM. Kernels that did not
change vary by up to ±30% between runs, so read only large ratios as real.
The raw data is in [perf/before.csv](perf/before.csv) and
[perf/after.csv](perf/after.csv).

### What profiling found

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

### Fixes

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

### Results (C++)

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

### Results (Python, `scripts/bench_python.py`)

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

### Valgrind summary after the fixes

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

### Left as is

- Dense matmul, LU and covariance are not cache blocked and do not
  auto-vectorize at `-O2`. At `-O2` GCC does not add runtime alias checks, and
  the output may alias the inputs. A `-O3 -march=native` build or a BLAS would
  speed them up, but portfolio problems in the book have tens of assets, so
  this code favours readability and has no dependencies.

## Round 2: callgrind and cachegrind across all chapters

Round 1 profiled the benchmark driver. This round ran callgrind over all 51
example and test binaries, then profiled every benchmark case on its own
with cache and branch simulation.

### Method

- `scripts/profile.sh chapters` gives total instructions per binary. About
  2 M of every run is the dynamic loader and iostream start-up, so the
  binaries well above that were looked at function by function.
- `scripts/profile.sh cases` runs `investm_bench --quick` under callgrind
  with `--cache-sim=yes --branch-sim=yes`. The driver calls
  `CALLGRIND_DUMP_STATS_AT` around each case, so each dump holds one case
  and none of the input setup. These client requests do nothing outside
  valgrind and compile away when `<valgrind/callgrind.h>` is missing.
- Caches are pinned to I1/D1 32 KB 8-way and LL 8 MB 16-way, 64 B lines,
  so a Docker Desktop run gives the same numbers as this one.
- Wall-clock: the old and new benchmark binaries were run alternately,
  three times each, and the best median per case was kept. The machine is a
  shared 4-core Xeon VM (GCC 13.3, `-O2 -g`). Kernels that did not change
  still move by about ±30%, so callgrind counts are the reliable measure.
  Wall-clock is only a check that the counts translate.

Raw data: [round2/callgrind_cases.csv](perf/round2/callgrind_cases.csv),
[round2/chapters.csv](perf/round2/chapters.csv),
[round2/timings_before.csv](perf/round2/timings_before.csv),
[round2/timings_after.csv](perf/round2/timings_after.csv).

### What profiling found

1. **European binomial pricing was O(n²) (Ch. 19).** `ch19_test` used
   52 M instructions, 77% of it in one test that prices a 2,000-step tree.
   Without early exercise, backward induction is just a binomial-weighted
   sum of terminal payoffs, which is O(n).
2. **The American tree called an out-of-line `Payoff` at every node** and
   branched on call or put inside the O(n²) loop.
3. **`EstimateTimeSeries` (Ch. 10) re-ran the full OLS for every
   security.** For each one it rebuilt the design matrix, transposed it, and
   formed and inverted X'X, even though the regressors are the same for all
   securities.
4. **Matrix multiply streamed `b` from L2 once per row of `a`.** At n = 200,
   cachegrind counted 1.02 M D1 misses for a 200×200 product.
5. **The covariance matrix re-subtracted the means inside the O(t·n²) loop**
   and streamed the n×n accumulator once per observation. That is 752 K D1
   misses for 1000×100 data.
6. **`Inverse` ran n column solves.** Each solve allocated a vector and wrote
   the result with stride n.
7. **The QL eigen step accounted for 65% of `EigenSymmetric`'s instructions
   and 60% of its D1 misses.** Each Givens rotation re-read and re-wrote two
   whole eigenvector rows. The tred2 inner loops also walked columns of the
   row-major matrix.
8. **The QP projector re-sorted its breakpoints from scratch in every FISTA
   iteration (Ch. 7/8).** In `tangency_no_short/100`, 10% of all branch
   mispredictions came from that sort.
9. **The strategy simulators (Ch. 23) grew two vectors with `push_back`
   without reserving.** `free` was 7.5% of `ch23_examples`, and the example
   copied and sorted each 2,000-path sample once per percentile.

The remaining large binaries (ch04, ch06, ch10_test, ch18) spend their time
in `std::normal_distribution` and `std::mt19937` generating test data.
That is not library code, so it was left alone.

### Fixes

| Area | Change |
|---|---|
| `ch19` | The European price is the binomial sum. Its weights are built by ratios outward from the mode, normalized by the total weight and cut off below `DBL_MIN`, so no factorials or powers are involved and nothing overflows. The American loop is specialized for calls and puts, with the payoff inlined. |
| `core/regression` | New `OlsDesign`: `Create(x)` builds the design matrix and (X'X)⁻¹ once, with X'X as row outer products and no transpose. `Fit(y)` is then O(T·K). `MultipleOls` is now a thin wrapper around it. |
| `ch10` | `EstimateTimeSeries` uses one `OlsDesign` for every security. |
| `core/matrix` `operator*` | 32×64 tiles of `b` (16 KB) stay in L1, and four rows of `b` are applied per pass over a row of `p`. |
| `core/matrix` `CovarianceMatrix` | Each observation is centred once. Four observations are applied per pass over the upper triangle. |
| `core/matrix` `Inverse` | Solves all right-hand sides at once, one contiguous row at a time. L⁻¹ is triangular, so the known zeros are skipped (n³/6 instead of n³/2). Columns are permuted at the end. |
| `core/matrix` `EigenSymmetric` | tred2 runs on the transpose, so its O(n³) loops walk rows. Each QL sweep records its rotations and applies them in one pass per eigenvector row, carrying the moving value in a register, with four rows interleaved. |
| `ch07` | `AffineSimplexProjector` keeps the breakpoint order from its previous call and insertion-sorts it, which takes about O(n) predictable comparisons because v changes little between iterations. It falls back to `std::sort` after 8n + 64 shifts. |
| `ch23` | Paths are reserved up front, and wealth is carried in a local variable. The example sorts each sample once. |
| Python | Same European binomial sum (log-space weights) and a single shared regression for the time-series factor model. |

**Results are unchanged, bit for bit, wherever the algorithm stayed the
same.** The new matmul, covariance and inverse code adds the same terms in
the same order as the old code. The rotation sweep does the same arithmetic
on each entry. The projector sorts by the same total order (key, then
index). New tests check this with `EXPECT_EQ`: the product against a naive
triple loop, the inverse against column-wise `Solve`, `OlsDesign` against
`MultipleOls`, and a reused projector against a fresh one, including ties
and large jumps. The O(n) European price is checked against plain backward
induction for 1 to 1,200 steps, calls and puts, several strikes, with a
dividend yield. The output of all 25 C++ example programs and all 25 Python
example modules is byte-identical to the previous version.

### Results: instructions and D1 misses per case (callgrind)

| Case | Instructions before → after | Ratio | D1 misses before → after |
|---|---|---:|---|
| `ch19/binomial_european/2000` | 20.2 M → 64 K | **314×** | 542 → 4 |
| `ch19/binomial_european/1000` | 5.12 M → 41 K | **125×** | 73 → 2 |
| `ch10/time_series/250x100x3` | 9.90 M → 2.59 M | **3.8×** | 58 K → 10 K |
| `core/matmul/200` | 64.9 M → 33.2 M | 2.0× | **1.02 M → 77 K** |
| `core/covariance_matrix/1000x100` | 42.2 M → 21.9 M | 1.9× | 752 K → 215 K |
| `core/multiple_ols/2000x10` | 3.08 M → 1.72 M | 1.8× | 56 K → 18 K |
| `ch19/binomial_american/2000` | 53.2 M → 32.2 M | 1.65× | 3.5 K → 3.4 K |
| `ch23/constant_mix/120` | 10.0 K → 6.1 K | 1.6× | |
| `ch23/cppi/120` | 14.5 K → 9.6 K | 1.5× | |
| `ch10/principal_components/250x100` | 39.2 M → 29.3 M | 1.33× | 376 K → 263 K |
| `core/eigen_symmetric/200` | 216 M → 178 M | 1.21× | 1.82 M → 1.86 M |
| `core/inverse/200` | 76.0 M → 66.6 M | 1.14× | 1.52 M → 1.07 M |
| `ch08/tangency_no_short/100` | 12.5 M → 11.0 M | 1.14× | unchanged; mispredicts 105 K → 49 K |
| `ch07/efficient_set_no_short/100x8` | 62.3 M → 56.2 M | 1.11× | unchanged; mispredicts 325 K → 184 K |
| Cholesky, LU solve, YTM, Black-Scholes, duration | | 1.00× | unchanged |

Per binary (`profile.sh chapters`): `ch23_examples` 112 M → 88 M,
`ch19_examples` 7.1 M → 4.4 M, `ch10_test` 10.9 M → 9.8 M. `ch19_test` and
`core_test` went *up* (52 M → 84 M and 40 M → 47 M) because the new tests
include deliberately simple O(n²) and O(n³) reference implementations.

### Results: wall-clock (best median of 3 alternating runs)

| Case | Before | After | Speed-up |
|---|---:|---:|---:|
| `ch19/binomial_european/2000` | 798 µs | 7.1 µs | **113×** |
| `ch19/binomial_european/1000` | 230 µs | 4.0 µs | **57×** |
| `ch10/time_series/250x100x3` | 564 µs | 162 µs | **3.5×** |
| `core/matmul/50` | 84 µs | 32 µs | 2.6× |
| `core/covariance_matrix/1000x100` | 2.96 ms | 1.32 ms | 2.2× |
| `ch19/binomial_american/1000` | 590 µs | 330 µs | 1.8× |
| `ch19/binomial_american/2000` | 2.18 ms | 1.26 ms | 1.7× |
| `core/inverse/200` | 6.30 ms | 3.93 ms | 1.6× |
| `core/matmul/200` | 3.09 ms | 1.98 ms | 1.6× |
| `core/multiple_ols/2000x10` | 213 µs | 138 µs | 1.5× |
| `ch10/principal_components/250x100` | 2.49 ms | 1.65 ms | 1.5× |
| `ch08/tangency_no_short/20` | 44.5 µs | 29.8 µs | 1.5× |
| `ch07/efficient_set_no_short/100x8` | 4.55 ms | 3.65 ms | 1.25× |
| `core/eigen_symmetric/200` | 12.2 ms | 11.7 ms | within noise |

Kernels with unchanged code moved between 0.8× and 1.4× across these runs
(for example `core/cholesky/200` came out at 1.38× and `ch14/duration_convexity`
at 0.79×), so speed-ups under about 1.4× here are not significant on their
own. The callgrind counts above are what show those changes.

### Valgrind summary after round 2

- **memcheck**: 0 errors and 0 leaks across all 25 example binaries and the
  benchmark driver.
- **massif**: peak heap 4.6 MB, still set by the benchmark's inputs (round
  1: 4.4 MB; the new `ch10/time_series` case adds one input panel).
- **DHAT**: 1,835 blocks and 15.5 MB in total for the whole quick run
  (it has 4 more cases than in round 1), with 0 bytes live at exit.
- **cachegrind**: matrix multiply and covariance are no longer L1-bound. The
  remaining D1 misses come from kernels that stream a matrix larger than
  32 KB once per step: `MultiplyInto` in the QP (an 80 KB covariance
  matrix per FISTA iteration at n = 100), the eigenvector update in QL, and
  LU and Cholesky at n = 200. The LL miss rate is 0.0%, so everything fits
  in L2/L3.

### Left as is

- The eigen-solver rewrite cut instructions by 17-21% (33% for PCA), but
  wall-clock is unchanged within noise. The QL sweep is now bound by the
  latency of its rotation chain, not by memory traffic. The arithmetic is
  the same as before, so the change was kept.
- Nothing auto-vectorizes at `-O2`. GCC 13's very-cheap cost model does not
  vectorize these loops even with `__restrict`, as checked with
  `-fopt-info-vec-all`. A `-O3 -march=native` build is the next lever if
  speed matters more than portability.
- Test-data generation (`std::normal_distribution`) dominates several test
  and example binaries. It is not library code.
