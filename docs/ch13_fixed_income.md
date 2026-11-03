# Chapter 13: Fixed-Income Securities

Code: [`cpp/ch13_fixed_income`](../cpp/ch13_fixed_income) and [`python/investm/ch13_fixed_income`](../python/investm/ch13_fixed_income).

| Section | Topic | C++ (`investm::ch13`) | Python |
|---|---|---|---|
| 13.2 | Money market yields (bills) | See Ch. 5 `BankDiscountYield`, `BondEquivalentYield` | |
| 13.3 | Treasury notes and bonds: coupon schedule, 32nds | `Bond`, `SurroundingCoupons`, `PriceFrom32nds` | `fixed_income.*` |
| 13.3 | Day counts: actual/actual, 30/360, actual/360 | `DayCountDays`, `AddMonths` | |
| 13.3 | Accrued interest, clean and dirty (invoice) prices | `AccruedInterest`, `CleanPrice`, `DirtyPrice`, `YieldFromCleanPrice` | |
| 13.5 | Corporate bonds on a 30/360 basis | `DayCount::kThirty360` | |
| 13.6 | Call provisions: yield-to-call and yield-to-worst | `YieldToCall`, `YieldToWorst` | |
| 13.8 | Preferred stock as a perpetuity | `PerpetuityValue`, `PerpetuityYield` | |
| 12.1.3 | Tax-exempt municipal bonds | See Ch. 12 `TaxableEquivalentYield` | |

## Notes

- Dates use `std::chrono::year_month_day` in C++ and `datetime.date` in Python.
- Pricing between coupon dates follows the street convention. Each remaining flow k is discounted by (1 + y/f)^(k + w), where w is the fraction of the current coupon period still to run. The clean price is the dirty price minus accrued interest. The examples show that the clean price is smooth through a coupon date, while the dirty price drops by the coupon.
- Yield-to-worst is the lowest of the yield-to-maturity and the yields to each call date. For a premium bond the worst case is usually an early call. For a discount bond it is usually maturity.
