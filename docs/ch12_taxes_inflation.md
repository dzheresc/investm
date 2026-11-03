# Chapter 12: Taxes and Inflation

Code: [`cpp/ch12_taxes_inflation`](../cpp/ch12_taxes_inflation) and [`python/investm/ch12_taxes_inflation`](../python/investm/ch12_taxes_inflation).

| Section | Topic | C++ (`investm::ch12`) | Python |
|---|---|---|---|
| 12.1 | Progressive tax schedules, marginal and average rates | `TaxSchedule` | `taxes.TaxSchedule` |
| 12.1.1 | After-tax returns, combined federal and state rates | `AfterTaxReturn`, `CombinedTaxRate` | `taxes.*` |
| 12.1.2 | The value of deferring tax on gains | `WealthTaxedAnnually`, `WealthTaxedAtEnd`, `WealthUntaxed`, `EffectiveDeferredRate` | |
| 12.1.3 | Tax-exempt bonds | `TaxableEquivalentYield`, `BreakevenTaxRate` | |
| 12.2.1 | Measuring inflation with a price index | `InflationRates`, `CumulativeInflation`, `PurchasingPower` | `inflation.*` |
| 12.2.2-12.2.3 | Real returns and the Fisher relation | `RealReturns`, `FisherNominalRate` | |
| 12.2.4 | Taxes on nominal returns | `RealAfterTaxReturn`, `EffectiveTaxOnRealReturn` | |
| 12.2.5 | Inflation-indexed bonds | `IndexedBondCashFlows`, `BreakevenInflation` | |
| 12.2.6 | Stocks as an inflation hedge | `InflationBeta` (OLS of nominal returns on inflation) | |

## Notes

- Tax brackets in the examples are made up. They show the mechanics only and are not any real year's schedule.
- Deferral matters because the tax on a deferred gain is paid once, on a compounded base. `EffectiveDeferredRate` turns this into an annual after-tax rate. That rate lies between r(1 − t) and r, and it rises with the holding period.
- A tax on nominal returns is a heavier tax on real returns. With 30% tax, a 10% nominal return and 6% inflation, the effective tax on the real return is 75%.
