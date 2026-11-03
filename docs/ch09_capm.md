# Chapter 9: The Capital Asset Pricing Model

Code: [`cpp/ch09_capm`](../cpp/ch09_capm) and [`python/investm/ch09_capm`](../python/investm/ch09_capm).

| Section | Topic | C++ (`investm::ch09`) | Python |
|---|---|---|---|
| 9.2 | The capital market line, separation theorem and market portfolio | `CapitalMarketLine`; examples show that the tangency portfolio equals the market weights | `capm.capital_market_line` |
| 9.3 | The security market line | `SecurityMarketLine`, `Beta`, `Betas`, `Alpha` | |
| 9.3 | Equilibrium expected returns | `ImpliedExpectedReturns` (reverse optimization) | |
| 9.4 | The market model and the partition of risk | `PartitionRisk`; Ch. 7 `FitMarketModel` estimates it | |
| App. A.1 | Restrictions on riskfree borrowing (zero-beta CAPM) | `ZeroBetaReturn`, `ZeroBetaSecurityMarketLine` | |
| App. A.2 | Heterogeneous expectations | Discussion only | |
| App. A.3 | Liquidity | `LiquidityAdjustedReturn` | |
| App. B | Derivation of the SML | Verified numerically in `capm_test`: with the market as the tangency portfolio, every security lies on the SML | |

## Notes

- `ImpliedExpectedReturns` goes in the opposite direction from Chapters 7 and 8. You start from market weights and a market risk premium and recover the expected returns under which those weights are the tangency portfolio. The unit tests check that the round trip works. This is the same idea that underlies equilibrium-based asset allocation.
- For the zero-beta CAPM, every frontier portfolio *p* except the minimum-variance portfolio has a zero-covariance partner *Z* on the frontier, with E_Z = (B·E_p − C)/(A·E_p − B). The SML then runs from E_Z to E_M.
