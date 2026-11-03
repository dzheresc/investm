# Investment Management Equations

A transcription of the 74-equation "Investment Management Equations"
cheat-sheet printed in Sharpe, Alexander and Bailey, *Investments*. The
numbering follows the sheet. Each equation names the C++ function
(namespace `investm::formulas`) and the Python function (package
`investm_formulas`) that implements it.

Rates and returns are decimals (5% is `0.05`).

## Reading notes

The photo was clear for almost every equation. These spots needed judgment:

| Eq. | What the photo shows | What is used here |
|---|---|---|
| 27 | The relation between $RR$ and $NR - CCL$ prints as a glyph that looks like `>`. | Read as $\approx$ (the usual approximation of the real rate). |
| 33 | $D_e = (P^+ - P^-)/(2P\Delta y)$. | Kept as printed. $P^+$ is taken as the price after the yield *falls* by $\Delta y$, and $P^-$ the price after it rises, so that $D_e > 0$. |
| 38 | The second term of the denominator shows $\left(\sum X_i\right)$ with no visible square. | Implemented as $\left(\sum X_i\right)^2$, the ordinary least squares slope. Without the square the formula is not dimensionally consistent. |
| 57 | The $d_1$ and $d_2$ numerators show `5σ²`; the decimal point is too faint to see. | Read as $0.5\sigma^2$. |
| 63 | The subscripts on the returns are small; read as $\bar r_C$ (chosen portfolio) and $\bar r_S$ (stock/risky portfolio). | As read. |

## Margin and returns (1 to 5)

**1. Rate of return on an asset or portfolio**

$$\text{Return} = \frac{\text{end-of-period wealth} - \text{beginning-of-period wealth}}{\text{beginning-of-period wealth}}$$

`RateOfReturn` / `rate_of_return`

**2. Actual margin in a stock purchase**

$$am = \frac{(n \times mp) - [(1 - im) \times pp \times n]}{n \times mp}$$

$n$ shares, $mp$ market price, $im$ initial margin, $pp$ purchase price.
`ActualMarginPurchase` / `actual_margin_purchase`

**3. Market price at which a margin purchaser will receive a margin call**

$$mp = \frac{(1 - im) \times pp}{1 - mm}$$

$mm$ maintenance margin. `MarginCallPricePurchase` / `margin_call_price_purchase`

**4. Actual margin in a short sale**

$$am = \frac{[(sp \times n) \times (1 + im)] - (mp \times n)}{mp \times n}$$

$sp$ short-sale price. `ActualMarginShortSale` / `actual_margin_short_sale`

**5. Market price at which a short seller will receive a margin call**

$$mp = \frac{sp \times (1 + im)}{1 + mm}$$

`MarginCallPriceShortSale` / `margin_call_price_short_sale`

## Portfolio theory, the market model and CAPM (6 to 20)

**6. Expected return on a portfolio**

$$\bar r_p = \sum_{i=1}^{N} X_i \bar r_i$$

`PortfolioExpectedReturn` / `portfolio_expected_return`

**7. Covariance between two securities**

$$\sigma_{ij} = \rho_{ij}\sigma_i\sigma_j$$

`Covariance` / `covariance`

**8. Standard deviation of a portfolio**

$$\sigma_p = \left[\sum_{i=1}^{N}\sum_{j=1}^{N} X_i X_j \sigma_{ij}\right]^{1/2}$$

`PortfolioStdDev` / `portfolio_std_dev`

**9. Standard deviation of a two-asset portfolio**

$$\sigma_p = \left[X_1^2\sigma_1^2 + X_2^2\sigma_2^2 + 2X_1X_2\rho_{12}\sigma_1\sigma_2\right]^{1/2}$$

`TwoAssetStdDev` / `two_asset_std_dev`

**10. The market model**

$$r_i = \alpha_{iI} + \beta_{iI} r_I + \varepsilon_{iI}$$

`MarketModelReturn` / `market_model_return`

**11. Beta from the market model**

$$\beta_{iI} = \frac{\sigma_{iI}}{\sigma_I^2}$$

`MarketModelBeta` / `market_model_beta`

**12. Security variance from the market model**

$$\sigma_i^2 = \beta_{iI}^2\sigma_I^2 + \sigma_{\varepsilon i}^2$$

`MarketModelVariance` / `market_model_variance`

**13. Security covariance from the market model**

$$\sigma_{ij} = \beta_{iI}\beta_{jI}\sigma_I^2$$

`MarketModelCovariance` / `market_model_covariance`

**14. Variance of a portfolio (by market and unique risk)**

$$\sigma_p^2 = \beta_{pI}^2\sigma_I^2 + \sigma_{\varepsilon p}^2$$

`PortfolioVarianceMarketModel` / `portfolio_variance_market_model`

**15. Market risk of a portfolio**

$$\beta_{pI} = \sum_{i=1}^{N} X_i\beta_{iI}$$

`PortfolioBeta` / `portfolio_beta`

**16. Unique risk of a portfolio**

$$\sigma_{\varepsilon p}^2 = \sum_{i=1}^{N} X_i^2\sigma_{\varepsilon i}^2$$

`PortfolioUniqueVariance` / `portfolio_unique_variance`

**17. Capital Market Line**

$$\bar r_p = r_f + \left[\frac{\bar r_M - r_f}{\sigma_M}\right]\sigma_p$$

`CapitalMarketLine` / `capital_market_line`

**18. Variance of the market portfolio**

$$\sigma_M^2 = \sum_{i=1}^{N}\sum_{j=1}^{N} X_{iM}X_{jM}\sigma_{ij}$$

`PortfolioVariance` / `portfolio_variance` (the same double sum as eq. 8,
with market weights)

**19. Security Market Line (covariance and beta versions)**

$$\bar r_i = r_f + \left[\frac{\bar r_M - r_f}{\sigma_M^2}\right]\sigma_{iM}$$

$$\bar r_i = r_f + (\bar r_M - r_f)\beta_{iM}$$

`SecurityMarketLineCov` / `security_market_line_cov`,
`SecurityMarketLine` / `security_market_line`

**20. Beta from the CAPM**

$$\beta_{iM} = \frac{\sigma_{iM}}{\sigma_M^2}$$

`CapmBeta` / `capm_beta`

## Factor models and APT (21 to 24)

**21. Two-factor model**

$$r_i = a_i + b_{i1}F_1 + b_{i2}F_2 + e_i$$

`TwoFactorReturn` / `two_factor_return`

**22. Variance of a security (by factor and non-factor risk, two-factor model)**

$$\sigma_i^2 = b_{i1}^2\sigma_{F1}^2 + b_{i2}^2\sigma_{F2}^2 + 2b_{i1}b_{i2}\,\mathrm{Cov}(F_1, F_2) + \sigma_{ei}^2$$

`TwoFactorVariance` / `two_factor_variance`

**23. Covariance between two securities (two-factor model)**

$$\sigma_{ij} = b_{i1}b_{j1}\sigma_{F1}^2 + b_{i2}b_{j2}\sigma_{F2}^2 + (b_{i1}b_{j2} + b_{i2}b_{j1})\,\mathrm{Cov}(F_1, F_2)$$

`TwoFactorCovariance` / `two_factor_covariance`

**24. Arbitrage Pricing Theory (two-factor model)**

$$\bar r_i = \lambda_0 + \lambda_1 b_{i1} + \lambda_2 b_{i2}$$

$$\bar r_i = r_f + (\delta_1 - r_f)b_{i1} + (\delta_2 - r_f)b_{i2}$$

`AptExpectedReturn` / `apt_expected_return`,
`AptExpectedReturnFromFactorPortfolios` / `apt_expected_return_from_factor_portfolios`

## Fixed income (25 to 34)

**25. Break-even tax rate for equivalence of tax-exempt and taxable bond yields**

$$t = 1 - \frac{\text{tax-exempt yield}}{\text{taxable yield}}$$

`BreakEvenTaxRate` / `break_even_tax_rate`

**26. Geometric mean (compound) annual growth rate of an index's value**

$$g = \left(\frac{C_e}{C_b}\right)^{1/y} - 1$$

$C_b$, $C_e$ beginning and ending values, $y$ years.
`GeometricMeanGrowthRate` / `geometric_mean_growth_rate`

**27. Real rate of return on an asset or portfolio**

$$RR = \left[\frac{1 + NR}{1 + CCL}\right] - 1$$

$$RR \approx NR - CCL$$

$CCL$ change in the cost of living.
`RealReturn` / `real_return`, `ApproxRealReturn` / `approx_real_return`

**28. Yield-to-maturity**

$$P_b = \frac{C_1}{(1+y)^1} + \frac{C_2}{(1+y)^2} + \cdots + \frac{C_N + M}{(1+y)^N}$$

`BondPrice` / `bond_price` gives $P_b$ for a yield;
`YieldToMaturity` / `yield_to_maturity` solves for $y$ by bisection.

**29. Discount factor**

$$d_t = \frac{1}{(1 + s_t)^t}$$

`DiscountFactor` / `discount_factor`

**30. Forward rate between years t−1 and t**

$$f_{t-1,t} = \frac{(1 + s_t)^t}{(1 + s_{t-1})^{t-1}} - 1$$

`ForwardRate` / `forward_rate`

**31. Duration**

$$D = \frac{\sum_{t=1}^{T} PV(C_t) \times t}{P_0}$$

`MacaulayDuration` / `macaulay_duration` (present values at the yield $y$)

**32. Modified duration**

$$D_m = \frac{D}{1 + y}$$

`ModifiedDuration` / `modified_duration`

**33. Effective duration**

$$D_e = \frac{P^+ - P^-}{2P\Delta y}$$

`EffectiveDuration` / `effective_duration` (see the reading notes)

**34. Relationship of duration to bond price changes**

$$\frac{\Delta P}{P} = -D\frac{\Delta y}{1 + y} = -D_m\Delta y$$

`PriceChangeFromDuration` / `price_change_from_duration`,
`PriceChangeFromModifiedDuration` / `price_change_from_modified_duration`

## Corporate finance and equity valuation (35 to 37, 44 to 54)

**35. Number of shares required to elect d directors under cumulative voting**

$$n = \frac{d \times s}{D + 1} + 1$$

$s$ shares voting, $D$ directors to be elected.
`SharesToElectDirectors` / `shares_to_elect_directors`

**36. Value of a right, rights-on**

$$R = \frac{C_o - S}{N + 1}$$

`RightValueRightsOn` / `right_value_rights_on`

**37. Value of a right, ex-rights**

$$R = \frac{C_e - S}{N}$$

`RightValueExRights` / `right_value_ex_rights`

**44. Beta of the firm**

$$\beta_{\text{firm}} = \beta_{\text{debt}}\left[\frac{(1 - \tau)D}{V_u}\right] + \beta_{\text{equity}}\left(\frac{E}{V_u}\right)$$

`FirmBeta` / `firm_beta`

**45. Beta of the firm's equity**

$$\beta_{\text{equity}} = \beta_{\text{firm}} + (\beta_{\text{firm}} - \beta_{\text{debt}})\left(\frac{D}{E}\right)(1 - \tau)$$

`EquityBeta` / `equity_beta`

**46. Adjusted historical beta**

$$\beta_a = a + b\beta_h$$

`AdjustedBeta` / `adjusted_beta`

**47. Capitalization-of-income method of valuation**

$$V = \sum_{t=1}^{\infty}\frac{C_t}{(1 + k)^t}$$

`CapitalizedValue` / `capitalized_value` (finite stream of cash flows)

**48. Zero-growth DDM**

$$V = \frac{D_1}{k}$$

`ZeroGrowthDdm` / `zero_growth_ddm`

**49. Constant-growth DDM**

$$V = \frac{D_1}{k - g}$$

`ConstantGrowthDdm` / `constant_growth_ddm`

**50. Multiple-growth DDM**

$$V = \left[\sum_{t=1}^{T}\frac{D_t}{(1 + k)^t}\right] + \left[\frac{D_{T+1}}{(k - g)(1 + k)^T}\right]$$

`MultipleGrowthDdm` / `multiple_growth_ddm`

**51. "Normal" price-earnings ratio with constant growth**

$$\frac{V}{E_0} = p\left(\frac{1 + r(1 - p)}{k - r(1 - p)}\right)$$

$p$ payout ratio, $r$ return on equity.
`NormalPriceEarnings` / `normal_price_earnings`

**52. Lintner dividend payout model**

$$D_t = ap^*E_t + (1 - a)D_{t-1}$$

`LintnerDividend` / `lintner_dividend`

**53. Annual earnings as a random walk**

$$E_t = E_{t-1} + \varepsilon_t$$

`RandomWalkEarnings` / `random_walk_earnings`

**54. Quarterly earnings as an autoregressive process**

(The sheet prints "autoprogressive".)

$$QE_t = QE_{t-4} + a(QE_{t-1} - QE_{t-5}) + b + e_t$$

`QuarterlyEarnings` / `quarterly_earnings`

## Simple linear regression (38 to 43)

All sums run over the $T$ observations $i = 1 \ldots T$ (the sheet writes
the upper limit as $N$).

**38. Beta from a simple linear regression**

$$\beta = \frac{\left(T \times \sum X_iY_i\right) - \left(\sum Y_i \times \sum X_i\right)}{\left(T \times \sum X_i^2\right) - \left(\sum X_i\right)^2}$$

`RegressionBeta` / `regression_beta` (see the reading notes on the square)

**39. Alpha from a simple linear regression**

$$\alpha = \frac{\sum Y_i}{T} - \frac{\beta \times \sum X_i}{T}$$

`RegressionAlpha` / `regression_alpha`

**40. Standard deviation of the random error term from a simple linear regression**

$$\sigma_\varepsilon = \left[\frac{\sum Y_i^2 - \left(\alpha \times \sum Y_i\right) - \left(\beta \times \sum X_iY_i\right)}{T - 2}\right]^{1/2}$$

`RegressionResidualStdDev` / `regression_residual_std_dev`

**41. Standard error of beta from a simple linear regression**

$$\sigma_\beta = \frac{\sigma_\varepsilon}{\left[\sum X_i^2 - \frac{\left(\sum X_i\right)^2}{T}\right]^{1/2}}$$

`RegressionBetaStdError` / `regression_beta_std_error`

**42. Standard error of alpha from a simple linear regression**

$$\sigma_\alpha = \frac{\sigma_\varepsilon}{\left[T - \frac{\left(\sum X_i\right)^2}{\sum X_i^2}\right]^{1/2}}$$

`RegressionAlphaStdError` / `regression_alpha_std_error`

**43. Correlation coefficient**

$$\rho = \frac{\left(T \times \sum X_iY_i\right) - \left(\sum Y_i \times \sum X_i\right)}{\left\{\left[\left(T \times \sum Y_i^2\right) - \left(\sum Y_i\right)^2\right] \times \left[\left(T \times \sum X_i^2\right) - \left(\sum X_i\right)^2\right]\right\}^{1/2}}$$

`CorrelationCoefficient` / `correlation_coefficient`

## Options and futures (55 to 60)

**55. Intrinsic value of puts and calls**

$$IV_C = \max(0, P_S - E) \qquad IV_P = \max(0, E - P_S)$$

`CallIntrinsicValue` / `call_intrinsic_value`,
`PutIntrinsicValue` / `put_intrinsic_value`

**56. Put-call parity**

$$P_P + P_S = P_C + \frac{E}{e^{RT}}$$

`PutFromParity` / `put_from_parity`, `CallFromParity` / `call_from_parity`

**57. Black-Scholes formula for the fair value of a call option**

$$V_C = N(d_1)P_S - \frac{E}{e^{RT}}N(d_2)$$

$$d_1 = \frac{\ln(P_S/E) + (R + 0.5\sigma^2)T}{\sigma\sqrt{T}}$$

$$d_2 = \frac{\ln(P_S/E) + (R - 0.5\sigma^2)T}{\sigma\sqrt{T}} = d_1 - \sigma\sqrt{T}$$

`BlackScholesD1`, `BlackScholesD2`, `BlackScholesCall` /
`black_scholes_d1`, `black_scholes_d2`, `black_scholes_call`

**58. Black-Scholes formula for the fair value of a put option**

$$P_P = \left(\frac{E}{e^{RT}}\right)N(-d_2) - P_SN(-d_1)$$

`BlackScholesPut` / `black_scholes_put`

**59. Fair value of a futures contract**

$$P_f = P_s + I - B + C$$

$I$ interest forgone, $B$ benefits of owning the asset, $C$ storage costs.
`FuturesFairValue` / `futures_fair_value`

**60. Fair value of an index futures contract**

$$P_f = P_s + RP_s - yP_s$$

`IndexFuturesFairValue` / `index_futures_fair_value`

## Investment companies (61 to 62)

**61. Investment company's net asset value**

$$NAV_t = \frac{MVA_t - LIAB_t}{NSO_t}$$

`NetAssetValue` / `net_asset_value`

**62. Return on an investment company's shares**

$$r_t = \frac{(NAV_t - NAV_{t-1}) + I_t + G_t}{NAV_{t-1}}$$

`InvestmentCompanyReturn` / `investment_company_return`

## Risk tolerance and performance evaluation (63 to 72)

**63. Risk tolerance (as implied by a choice of a portfolio)**

$$\tau = \frac{2(\bar r_C - r_f)\sigma_S^2}{(\bar r_S - r_f)^2}$$

`ImpliedRiskTolerance` / `implied_risk_tolerance`

**64. Certainty equivalent return**

$$u_i = \bar r_p - \left(\frac{1}{\tau}\right)\sigma_p^2$$

`CertaintyEquivalentReturn` / `certainty_equivalent_return`

**65. Ex post alpha**

$$\alpha_p = ar_p - ar_{bp}$$

`ExPostAlpha` / `ex_post_alpha`

**66. Ex post alpha based on the CAPM**

$$\alpha_p = ar_p - [ar_f + (ar_M - ar_f)\beta_p]$$

`CapmExPostAlpha` / `capm_ex_post_alpha`

**67. Ex post characteristic line**

$$r_p - r_f = \alpha_p + \beta_p(r_M - r_f)$$

`CharacteristicLine` / `characteristic_line` (returns $r_p - r_f$)

**68. Reward-to-volatility ratio (Treynor ratio)**

$$RVOL_p = \frac{ar_p - ar_f}{\beta_p}$$

`TreynorRatio` / `treynor_ratio`

**69. Sharpe ratio**

$$SR_p = \frac{ar_p - ar_f}{\sigma_p}$$

`SharpeRatio` / `sharpe_ratio`

**70. M²**

$$M_p^2 = ar_f + \left(\frac{ar_p - ar_f}{\sigma_p}\right)\sigma_M$$

`MSquared` / `m_squared`

**71. Ex post characteristic curve**

$$r_p - r_f = a + b(r_M - r_f) + c[(r_M - r_f)^2]$$

`QuadraticCharacteristicCurve` / `quadratic_characteristic_curve`

**72. Ex post characteristic lines**

$$r_p - r_f = a + b(r_M - r_f) + c[D(r_M - r_f)]$$

where $D = 0$ if $r_M > r_f$ and $D = -1$ if $r_M < r_f$.
`DummyCharacteristicLines` / `dummy_characteristic_lines`

## International investing (73 to 74)

**73. Return on a foreign investment**

$$r_F = r_d + r_e + r_dr_e$$

$r_d$ return in the foreign currency, $r_e$ change in the exchange rate.
`ForeignReturn` / `foreign_return`

**74. Standard deviation of a foreign investment**

$$\sigma_F = \left(\sigma_d^2 + \sigma_e^2 + 2\rho_{de}\sigma_d\sigma_e\right)^{1/2}$$

`ForeignStdDev` / `foreign_std_dev`
