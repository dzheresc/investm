# Investment Management Equations

The 74 numbered equations from the "Investment Management Equations"
cheat-sheet in Sharpe, Alexander and Bailey, *Investments*, transcribed and
implemented as a small library in C++20 and Python 3.

- [EQUATIONS.md](EQUATIONS.md) has every equation in LaTeX, numbered as on
  the sheet, with the matching function names and notes on the few spots
  that were hard to read in the photo.
- `cpp/` is the C++20 library (namespace `investm::formulas`, umbrella
  header `investm/formulas.h`) with GoogleTest tests.
- `python/` is the `investm_formulas` package (standard library only) with
  pytest tests.

Both follow the Google Style Guide. Rates and returns are decimals.

## Modules

| Module | Equations |
|---|---|
| `margin` | 1 to 5: return and margin accounts |
| `portfolio` | 6 to 20: portfolio risk, market model, CML, SML |
| `factor_models` | 21 to 24: two-factor model and APT |
| `fixed_income` | 25 to 34: taxes, real returns, bond pricing, duration |
| `regression` | 38 to 43: simple linear regression |
| `equity` | 35 to 37, 44 to 54: voting, rights, firm betas, DDMs, earnings |
| `derivatives` | 55 to 60: option values, Black-Scholes, futures |
| `performance` | 61 to 74: investment companies, risk tolerance, performance measures, foreign investing |

## Build and test on Ubuntu

```sh
sudo apt-get install -y cmake g++ libgtest-dev
cmake -S formulas/cpp -B build
cmake --build build -j
ctest --test-dir build --output-on-failure

cd formulas/python
python3 -m pip install pytest
python3 -m pytest
```

If GoogleTest is not installed, CMake downloads it with `FetchContent`.

## Example

```cpp
#include "investm/formulas.h"

double call = investm::formulas::BlackScholesCall(100, 100, 0.05, 1.0, 0.2);
```

```python
import investm_formulas as f

call = f.black_scholes_call(100, 100, 0.05, 1.0, 0.2)
```
