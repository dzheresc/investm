# Chapter 5: The Valuation of Riskless Securities

Code: [`cpp/ch05_riskless_securities`](../cpp/ch05_riskless_securities) and [`python/investm/ch05_riskless_securities`](../python/investm/ch05_riskless_securities).

The fixed-income chapters (13-15) build on this chapter.

| Section | Topic | C++ (`investm::ch05`) | Python |
|---|---|---|---|
| 5.1 | Nominal versus real interest rates | `RealRate`, `NominalRate` (`rates.h`) | `rates.real_rate` |
| 5.2, 5.2.1 | Yield-to-maturity | `CouponBond`, `PresentValueAtYield`, `YieldToMaturity`, `InternalRateOfReturn` (`cash_flows.h`) | `cash_flows.*` |
| 5.3 | Spot rates | `SpotCurve`, `Bootstrap` (`term_structure.h`) | `term_structure.SpotCurve`, `bootstrap` |
| 5.4 | Discount factors | `SpotCurve::DiscountFactor`, `SpotCurve::PresentValue` | |
| 5.5 | Forward rates | `SpotCurve::Forward(a, b)`, `SpotCurve::OneYearForward(t)` | |
| 5.6 | Forward rates and discount factors | `SpotCurve::FromForwards`; the forward rate is (d_a/d_b)^(1/(b−a)) − 1 | |
| 5.7 | Compounding | `EffectiveAnnualRate`, `AprFromEffective`, `ConvertCompounding` | |
| 5.8 | The bank discount method | `BankDiscountYield`, `BondEquivalentYield`, `MoneyMarketYield` | |
| 5.9 | Yield curves | `SpotCurve::ParYield` | |
| 5.10.1 | Unbiased expectations theory | `ExpectedFutureOneYearRates`, `RollForwardOneYear` | |
| 5.10.2 | Liquidity preference theory | `LiquidityPreferenceCurve`, `ImpliedLiquidityPremiums` | |
| 5.10.3-5.10.5 | Segmentation, preferred habitat, evidence | Discussion only. Under segmentation no arbitrage relation links the maturities, so there is nothing to compute. | |
| 5.11 | Holding-period return | `HoldingPeriodReturn`, `OneYearHprOfZero` | |
| Appendix | Continuous compounding | `EffectiveFromContinuous`, `ContinuousFromEffective`, `PresentValueContinuous` | |

## Conventions

`SpotCurve` uses whole-year maturities and annual compounding, so you can check every number by hand. The YTM functions accept any coupon frequency.

## The relationships to remember

    d_t          = 1 / (1 + s_t)^t
    (1 + s_t)^t  = (1 + s_1)(1 + f_{1,2}) … (1 + f_{t−1,t})
    1 + f_{a,b}  = (d_a / d_b)^(1/(b−a))
    bond price   = Σ CF_t · d_t

YTM is the single rate that makes Σ CF_t/(1+y)^t equal the price. In effect it is an average of the spot rates, weighted by the bond's cash flows. The examples show two 5-year bonds, both priced correctly off the same spot curve, that have different YTMs because their coupons differ.

The holding-period example illustrates the unbiased expectations theory. Suppose next year's curve turns out to be exactly what today's forward rates implied. Then every zero-coupon bond earns the 1-year spot rate over the year, whatever its maturity.
