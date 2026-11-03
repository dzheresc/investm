"""Voting, rights, betas of the firm and equity valuation.

Equations 35 to 37 and 44 to 54.
"""

import math
from typing import Sequence

__all__ = [
    "shares_to_elect_directors",
    "right_value_rights_on",
    "right_value_ex_rights",
    "firm_beta",
    "equity_beta",
    "adjusted_beta",
    "capitalized_value",
    "zero_growth_ddm",
    "constant_growth_ddm",
    "multiple_growth_ddm",
    "normal_price_earnings",
    "lintner_dividend",
    "random_walk_earnings",
    "quarterly_earnings",
]


def shares_to_elect_directors(directors_wanted: float, shares_voting: float,
                              directors_elected: float) -> float:
    """Eq. 35: n = d s / (D + 1) + 1 under cumulative voting."""
    return directors_wanted * shares_voting / (directors_elected + 1.0) + 1.0


def right_value_rights_on(stock_price: float, subscription_price: float,
                          rights_per_share: float) -> float:
    """Eq. 36: R = (C_o - S) / (N + 1)."""
    return (stock_price - subscription_price) / (rights_per_share + 1.0)


def right_value_ex_rights(stock_price: float, subscription_price: float,
                          rights_per_share: float) -> float:
    """Eq. 37: R = (C_e - S) / N."""
    return (stock_price - subscription_price) / rights_per_share


def firm_beta(beta_debt: float, beta_equity_value: float, tax_rate: float,
              debt: float, equity: float, unlevered_value: float) -> float:
    """Eq. 44: beta_firm = beta_debt (1-tau) D / V_u + beta_equity E / V_u."""
    return (beta_debt * (1.0 - tax_rate) * debt / unlevered_value +
            beta_equity_value * equity / unlevered_value)


def equity_beta(beta_firm: float, beta_debt: float, debt: float,
                equity: float, tax_rate: float) -> float:
    """Eq. 45: beta_equity = beta_firm + (beta_firm - beta_debt)(D/E)(1-tau)."""
    return beta_firm + (beta_firm - beta_debt) * (debt / equity) * \
        (1.0 - tax_rate)


def adjusted_beta(a: float, b: float, historical_beta: float) -> float:
    """Eq. 46: beta_a = a + b beta_h."""
    return a + b * historical_beta


def capitalized_value(cash_flows: Sequence[float], discount_rate: float) -> float:
    """Eq. 47: V = sum C_t / (1+k)^t, for a finite stream C_1 .. C_T."""
    return math.fsum(c / (1.0 + discount_rate)**t
                     for t, c in enumerate(cash_flows, start=1))


def zero_growth_ddm(next_dividend: float, discount_rate: float) -> float:
    """Eq. 48: V = D_1 / k."""
    return next_dividend / discount_rate


def constant_growth_ddm(next_dividend: float, discount_rate: float,
                        growth: float) -> float:
    """Eq. 49: V = D_1 / (k - g), requires k > g."""
    if discount_rate <= growth:
        raise ValueError("discount rate must exceed growth rate")
    return next_dividend / (discount_rate - growth)


def multiple_growth_ddm(dividends: Sequence[float], dividend_after: float,
                        discount_rate: float, growth: float) -> float:
    """Eq. 50: PV of D_1 .. D_T plus D_{T+1} / ((k - g)(1+k)^T)."""
    if discount_rate <= growth:
        raise ValueError("discount rate must exceed growth rate")
    t = len(dividends)
    return capitalized_value(dividends, discount_rate) + dividend_after / (
        (discount_rate - growth) * (1.0 + discount_rate)**t)


def normal_price_earnings(payout: float, return_on_equity: float,
                          discount_rate: float) -> float:
    """Eq. 51: V/E_0 = p (1 + r(1-p)) / (k - r(1-p))."""
    g = return_on_equity * (1.0 - payout)
    return payout * (1.0 + g) / (discount_rate - g)


def lintner_dividend(adjustment: float, target_payout: float, earnings: float,
                     previous_dividend: float) -> float:
    """Eq. 52: D_t = a p* E_t + (1 - a) D_{t-1}."""
    return adjustment * target_payout * earnings + \
        (1.0 - adjustment) * previous_dividend


def random_walk_earnings(previous_earnings: float, shock: float) -> float:
    """Eq. 53: E_t = E_{t-1} + epsilon_t."""
    return previous_earnings + shock


def quarterly_earnings(qe_lag4: float, qe_lag1: float, qe_lag5: float,
                       a: float, b: float, error: float = 0.0) -> float:
    """Eq. 54: QE_t = QE_{t-4} + a (QE_{t-1} - QE_{t-5}) + b + e_t."""
    return qe_lag4 + a * (qe_lag1 - qe_lag5) + b + error
