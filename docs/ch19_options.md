# Chapter 19: Options

Code: [`cpp/ch19_options`](../cpp/ch19_options) and [`python/investm/ch19_options`](../python/investm/ch19_options).

| Section | Topic | C++ (`investm::ch19`) | Python |
|---|---|---|---|
| 19.1-19.2 | Payoffs and profits at expiry; strategies built from legs | `Payoff`, `Leg`, `StrategyPayoff`, `StrategyProfit` | `payoffs.*` |
| 19.3 | Put-call parity and no-arbitrage bounds | `ParityPutPrice`, `ParityCallPrice`, `ParityDeviation`, `CallLowerBound`, `PutLowerBound` | |
| 19.4.1 | One-period binomial model: replication and the hedge ratio | `OnePeriodBinomial` | `pricing.*` |
| 19.4.2 | Multi-period binomial trees, European and American | `BinomialPrice` | |
| 19.5 | Black-Scholes with a dividend yield; the Greeks | `BlackScholesPrice`, `BlackScholesGreeks` | |
| 19.5.4 | Implied and historical volatility | `ImpliedVolatility`, `HistoricalVolatility` | |
| 19.7 | Warrants and dilution | `WarrantValue` | |

## Notes

- The binomial tree uses the Cox-Ross-Rubinstein parameterization u = e^{σ√Δt}, d = 1/u. Tests check convergence to Black-Scholes, and that early exercise of an American call on a non-dividend stock adds nothing.
- Every analytic Greek is checked against a central finite difference. Theta is per year and vega per unit of volatility. Divide by 365 or 100 for "per day" or "per vol point" quotes.
- `ImpliedVolatility` first checks the no-arbitrage range, then runs Newton's method on vega from the inflection point of the price in volatility, inside a bracket that narrows with each step; Brent's method on that bracket is the fallback.
