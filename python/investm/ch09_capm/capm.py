"""Chapter 9: CML, SML, betas, risk partition, zero-beta CAPM.

Mirrors cpp/ch09_capm/capm.h.
"""

import numpy as np


def capital_market_line(rf: float, market_return: float, market_std_dev: float,
                        std_dev: float) -> float:
    if market_std_dev <= 0:
        raise ValueError("market must be risky")
    return rf + (market_return - rf) / market_std_dev * std_dev


def security_market_line(rf: float, market_return: float, b: float) -> float:
    return rf + (market_return - rf) * b


def beta(cov_with_market: float, market_variance: float) -> float:
    if market_variance <= 0:
        raise ValueError("market must be risky")
    return cov_with_market / market_variance


def betas(cov, market_weights) -> np.ndarray:
    """beta = S w_M / (w_M' S w_M)."""
    w = np.asarray(market_weights, float)
    c = np.asarray(cov, float) @ w
    return c / float(w @ c)


def alpha(expected_return: float, rf: float, market_return: float,
          b: float) -> float:
    return expected_return - security_market_line(rf, market_return, b)


def implied_expected_returns(cov, market_weights, rf: float,
                             market_risk_premium: float) -> np.ndarray:
    return rf + betas(cov, market_weights) * market_risk_premium


def partition_risk(total_variance: float, b: float,
                   market_variance: float) -> tuple[float, float]:
    """(market variance, non-market variance)."""
    market = b * b * market_variance
    return market, total_variance - market


def zero_beta_return(a: float, b: float, c: float,
                     frontier_return: float) -> float:
    """E_Z = (B E_p - C) / (A E_p - B) for frontier constants A, B, C."""
    denom = a * frontier_return - b
    if denom == 0:
        raise ValueError("minimum-variance portfolio has no zero-beta partner")
    return (b * frontier_return - c) / denom


def zero_beta_security_market_line(zero_beta: float, market_return: float,
                                   b: float) -> float:
    return zero_beta + (market_return - zero_beta) * b


def liquidity_adjusted_return(rf: float, market_return: float, b: float,
                              trading_cost: float,
                              cost_premium: float) -> float:
    return security_market_line(rf, market_return,
                                b) + cost_premium * trading_cost
