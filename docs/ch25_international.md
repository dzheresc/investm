# Chapter 25: International Investing

Code: [`cpp/ch25_international`](../cpp/ch25_international) and [`python/investm/ch25_international`](../python/investm/ch25_international).

| Section | Topic | C++ (`investm::ch25`) | Python |
|---|---|---|---|
| 25.3 | Returns in domestic currency and currency risk | `CurrencyReturn`, `DomesticReturn`, `DomesticReturns`, `DomesticReturnVariance` | `international.*` |
| 25.4 | Covered interest parity and forward premiums | `ParityForward`, `ForwardPremium` | |
| 25.4 | Hedging currency risk with forwards | `HedgedReturn` | |
| 25.5 | Purchasing power parity, real exchange rates | `PppExpectedChange`, `RealExchangeRate` | |
| 25.6 | Diversification across markets | `TwoMarketStdDev`, `MinimumRiskForeignWeight` | |

## Notes

- Exchange rates are quoted as domestic currency per unit of foreign currency.
- With covered interest parity, a fully hedged foreign deposit earns exactly the domestic rate. For a stock, the hedge locks in only the expected value. The surprise part of the return is still converted at the realized spot rate.
