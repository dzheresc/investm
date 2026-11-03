# Chapter 3: Security Markets

Code: [`cpp/ch03_security_markets`](../cpp/ch03_security_markets) and [`python/investm/ch03_security_markets`](../python/investm/ch03_security_markets).

| Section | Topic | C++ (`investm::ch03`) | Python |
|---|---|---|---|
| 3.1.1 | Call markets | `ClearCallMarket`, `DemandAt`, `SupplyAt` (`call_market.h`) | `call_market.clear_call_market` |
| 3.1.2 | Continuous markets | See Ch. 2's `Simulate`. Each order fills against the standing quote as it arrives. | |
| 3.2-3.6, 3.9 | Exchanges, OTC, clearing, insurance, regulation | Institutional material, so no code. | |
| 3.7 | Commissions | `CommissionSchedule` (`costs.h`) | `costs.CommissionSchedule` |
| 3.8.1 | Bid-ask spread | `Quote`, `EffectiveHalfSpread` | `costs.Quote` |
| 3.8.2 | Price impact | `SqrtPriceImpact`, `OneWayCost` | `costs.sqrt_price_impact` |

A call auction sets one price for the whole batch of orders: the price at which the most shares can trade. Its demand schedule slopes down and its supply schedule slopes up. Chapter 4.1 uses the same picture to explain how prices form.

The square-root price-impact model is an industry rule of thumb, not something taken from the book. It is included because the book's point, that large orders move prices against the trader, needs a functional form before you can put a number on it.
