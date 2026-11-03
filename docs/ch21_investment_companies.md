# Chapter 21: Investment Companies

Code: [`cpp/ch21_investment_companies`](../cpp/ch21_investment_companies) and [`python/investm/ch21_investment_companies`](../python/investm/ch21_investment_companies).

| Section | Topic | C++ (`investm::ch21`) | Python |
|---|---|---|---|
| 21.2 | Net asset value | `NetAssetValue` | `funds.*` |
| 21.3 | Closed-end funds: premiums and discounts | `PremiumOrDiscount` | |
| 21.4 | Open-end funds: offering price and loads | `OfferingPrice`, `LoadAsFractionOfInvestment` | |
| 21.4 | Fund returns with distributions; reinvestment | `FundReturn`, `SharesAfterReinvestment` | |
| 21.5 | Expense ratios, front and back-end loads | `TerminalValue`, `AnnualizedFeeDrag` | |
| 21.5 | When does a load fund beat a no-load fund? | `BreakevenYears` | |

## Notes

- A front load quoted on the offering price costs more than it looks: 5.75% of the offering price is 6.10% of the money actually invested.
- `AnnualizedFeeDrag` turns loads and expense ratios into one annual number. A load matters most for short holding periods, and the expense ratio dominates over long ones.
