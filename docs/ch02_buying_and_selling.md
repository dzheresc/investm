# Chapter 2: Buying and Selling Securities

Code: [`cpp/ch02_buying_and_selling`](../cpp/ch02_buying_and_selling) and [`python/investm/ch02_buying_and_selling`](../python/investm/ch02_buying_and_selling).

| Section | Topic | C++ (`investm::ch02`) | Python |
|---|---|---|---|
| 2.1-2.2 | Order size and time limit | Not modelled. `Simulate` treats every order as good-till-cancelled over the whole path. | |
| 2.3.1-2.3.4 | Market, limit, stop and stop-limit orders | `Order`, `Simulate` (`orders.h`) | `orders.simulate` |
| 2.4.1 | Margin purchases | `MarginPurchase` (`margin.h`) | `margin.MarginPurchase` |
| 2.4.2 | Short sales | `ShortSale` | `margin.ShortSale` |
| 2.4.3 | Securities lending | No code. Lending is the mechanism that makes a short sale possible. | |
| 2.4.4 | Aggregation | `Account` | `margin.Account` |

## Key formulas

**Margin purchase.** Take *n* shares at price *P*, bought with a loan *L*. The actual margin is (nP − L)/(nP), and the margin call comes at

    P_call = L / (n·(1 − mm))

**Short sale.** Let *A* be the short-sale proceeds plus the investor's deposit, both held by the broker. The actual margin is (A − nP)/(nP), and the margin call comes at

    P_call = A / (n·(1 + mm))

**Leverage.** Buying on margin scales the return on your own money by roughly 1/(initial margin), minus the loan interest. The examples print the levered and unlevered returns side by side.
