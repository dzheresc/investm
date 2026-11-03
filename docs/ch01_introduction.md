# Chapter 1: Introduction

Code: [`cpp/ch01_introduction`](../cpp/ch01_introduction) and [`python/investm/ch01_introduction`](../python/investm/ch01_introduction).

Most of this chapter describes the investment environment and the investment process in words. The code covers the one quantitative idea it introduces.

| Section | Topic | C++ (`investm::ch01`) | Python (`investm.ch01_introduction`) |
|---|---|---|---|
| 1.1.2 | Risk, return, and diversification | `HoldingPeriodReturn`, `SimpleReturns`, `CumulativeReturn`, `Annualize`, `EqualWeightPortfolioStdDev` | `returns.holding_period_return`, ... |
| 1.2 | The investment process | No code. Later chapters map onto its steps: security analysis (Ch. 16-18, 22), portfolio construction (Ch. 6-8), revision and evaluation (Ch. 23-24). | |

## Diversification

Suppose you hold N assets in equal weights, and every asset has the same volatility σ and the same pairwise correlation ρ. The portfolio variance is

    σ²_p = σ²/N + (1 − 1/N)·ρ·σ²

The first term is unique risk, and it shrinks like 1/N. The second term is market-wide risk. It converges to ρσ², which no amount of diversification removes. Chapter 7.4 restates this result with the market model and betas.

When you run the examples, σ_p falls from 40% toward a floor of 20%.
