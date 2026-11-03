# investm

A code companion to *Investments* by William F. Sharpe, Gordon J. Alexander and Jeffery V. Bailey. It has one module per chapter, written in both **C++20** and **Python 3**, each with worked examples and unit tests.

All code and explanations here are original. They implement the quantitative ideas of each chapter; the book's text is not reproduced. Section numbers follow the book's table of contents, so you can read the two side by side.

## Layout

```
cpp/
  core/                      root finding, statistics, linear algebra, OLS, tables
  chNN_<topic>/              one static library per chapter
    *.h, *.cc                library code (namespace investm::chNN)
    *_test.cc                GoogleTest unit tests
    examples_main.cc         worked examples -> chNN_examples binary
python/
  investm/core/              same helpers in Python (numpy from Chapter 6 on)
  investm/chNN_<topic>/      one package per chapter
    *.py, *_test.py          library code and unittest/pytest tests
    examples.py              python -m investm.chNN_<topic>.examples
docs/chNN_<topic>.md         section-by-section map and notes for both languages
docker/Dockerfile            Ubuntu 24.04 toolchain (gcc, cmake, gtest, valgrind, python)
```

Both implementations follow the [Google Style Guides](https://google.github.io/styleguide/). Formatting uses `.clang-format` (Google) and yapf `--style=google`. Lint uses cpplint and pylint. In line with the C++ guide there are no exceptions: violated preconditions abort through `INVESTM_CHECK`, and operations that can legitimately fail return `std::optional`. The Python versions raise `ValueError` or return `None` to match.

## Building and testing on Ubuntu

```bash
sudo apt-get install -y build-essential cmake libgtest-dev python3 python3-numpy python3-pip valgrind
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j
ctest --test-dir build --output-on-failure      # unit tests + every examples binary
./build/cpp/ch05_riskless_securities/ch05_examples

cd python
python3 -m pytest                               # or: python3 -m unittest discover -p '*_test.py'
python3 -m investm.ch05_riskless_securities.examples
```

If `libgtest-dev` is not installed, CMake downloads GoogleTest automatically.

`scripts/check.sh` runs everything that CI runs: format check, cpplint, build with `-Werror`, ctest, pytest and pylint.

## Building on Windows with Docker Desktop

Clone the repository into `C:\develop\investm`, then run:

```powershell
docker build -t investm-dev -f docker/Dockerfile .
docker run --rm -it -v C:\develop:/develop -w /develop/investm investm-dev scripts/check.sh
```

The container mounts `C:\develop` at `/develop`, so build outputs, valgrind reports and so on land back on the Windows disk.

## Chapter map

| Part | Ch. | Topic | Status |
|---|---|---|---|
| I. Introduction | 1 | Introduction: return, risk, diversification | done |
| | 2 | Buying and selling securities: orders, margin, short sales | done |
| | 3 | Security markets: call auctions, spreads, commissions, price impact | done |
| | 4 | Efficient markets: supply/demand schedules, event studies, random-walk tests | done |
| | 5 | Riskless securities: YTM, spot/forward rates, bootstrapping, term-structure theories | done |
| II. Portfolio analysis | 6 | Portfolio selection problem: utility, indifference curves, portfolio moments | done |
| | 7 | Portfolio analysis: Markowitz efficient set, market model, diversification | done |
| | 8 | Riskfree borrowing and lending | done |
| III. Equilibrium | 9 | CAPM: CML, SML, beta | done |
| | 10 | Factor models | done |
| | 11 | Arbitrage pricing theory | done |
| | 12 | Taxes and inflation | done |
| IV. Fixed income | 13 | Fixed-income securities: day counts, accrued interest, callables | done |
| | 14 | Bond analysis: duration, convexity, default risk, credit scoring | done |
| | 15 | Bond portfolio management: horizon analysis, immunization, dedication, swaps | done |
| V. Common stocks | 16 | Common stocks: voting, splits, rights, betas and leverage | done |
| | 17 | Valuation of common stocks: dividend discount models, P/E, PVGO | done |
| | 18 | Earnings: Lintner model, naive forecasts, SUE | done |
| VI. Derivatives | 19 | Options: strategies, parity, binomial, Black-Scholes, Greeks, warrants | done |
| | 20 | Futures: marking to market, cost of carry, hedging, Black's model | done |
| VII. Investment management | 21 | Investment companies | planned |
| | 22 | Financial analysis | planned |
| | 23 | Investment management | planned |
| | 24 | Portfolio performance evaluation | planned |
| | 25 | International investing | planned |
