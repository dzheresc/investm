# Chapter 20: Futures

Code: [`cpp/ch20_futures`](../cpp/ch20_futures) and [`python/investm/ch20_futures`](../python/investm/ch20_futures).

| Section | Topic | C++ (`investm::ch20`) | Python |
|---|---|---|---|
| 20.2 | Margin and marking to market | `MarkToMarket` | `futures.*` |
| 20.3 | Basis and convergence | `Basis`; examples | |
| 20.4 | Minimum-variance hedging and hedge effectiveness | `MinimumVarianceHedge`, `HedgeEffectiveness`, `ContractsToHedge` | |
| 20.4 | Changing portfolio beta with stock-index futures | `IndexFuturesContracts` | |
| 20.5 | Cost of carry, stock-index futures with discrete dividends | `CostOfCarryPrice`, `IndexFuturesPrice` | |
| 20.5 | Arbitrage band with unequal rates and trading costs | `NoArbitrageBand` | |
| 20.6 | Options on futures (Black's model) | `BlackFuturesOptionPrice` | |

## Notes

- In `MarkToMarket`, a margin call restores equity to the initial margin, not just to the maintenance level. This is the usual exchange convention.
- With equal borrowing and lending rates and no costs, the arbitrage band collapses to the cost-of-carry price.
- Black's formula equals Black-Scholes on the underlying with a dividend yield equal to r, because F = S e^{rT}. A test checks this.
