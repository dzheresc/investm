"""Investment companies, risk tolerance, performance and foreign investing.

Equations 61 to 74.
"""

import math

__all__ = [
    "net_asset_value",
    "investment_company_return",
    "implied_risk_tolerance",
    "certainty_equivalent_return",
    "ex_post_alpha",
    "capm_ex_post_alpha",
    "characteristic_line",
    "treynor_ratio",
    "sharpe_ratio",
    "m_squared",
    "quadratic_characteristic_curve",
    "dummy_characteristic_lines",
    "foreign_return",
    "foreign_std_dev",
]


def net_asset_value(market_value_assets: float, liabilities: float,
                    shares_outstanding: float) -> float:
    """Eq. 61: NAV_t = (MVA_t - LIAB_t) / NSO_t."""
    return (market_value_assets - liabilities) / shares_outstanding


def investment_company_return(nav: float, previous_nav: float, income: float,
                              capital_gains: float) -> float:
    """Eq. 62: r_t = (NAV_t - NAV_{t-1} + I_t + G_t) / NAV_{t-1}."""
    return (nav - previous_nav + income + capital_gains) / previous_nav


def implied_risk_tolerance(chosen_return: float, stock_return: float,
                           risk_free: float, stock_std_dev: float) -> float:
    """Eq. 63: tau = 2 (r_C - r_f) sigma_S^2 / (r_S - r_f)^2."""
    return 2.0 * (chosen_return - risk_free) * stock_std_dev**2 / \
        (stock_return - risk_free)**2


def certainty_equivalent_return(expected_return: float, std_dev: float,
                                risk_tolerance: float) -> float:
    """Eq. 64: u = r_p - (1 / tau) sigma_p^2."""
    return expected_return - std_dev**2 / risk_tolerance


def ex_post_alpha(portfolio_return: float, benchmark_return: float) -> float:
    """Eq. 65: alpha_p = ar_p - ar_bp."""
    return portfolio_return - benchmark_return


def capm_ex_post_alpha(portfolio_return: float, risk_free: float,
                       market_return: float, beta: float) -> float:
    """Eq. 66: alpha_p = ar_p - [ar_f + (ar_M - ar_f) beta_p]."""
    return portfolio_return - (risk_free +
                               (market_return - risk_free) * beta)


def characteristic_line(alpha: float, beta: float, market_return: float,
                        risk_free: float) -> float:
    """Eq. 67: r_p - r_f = alpha_p + beta_p (r_M - r_f)."""
    return alpha + beta * (market_return - risk_free)


def treynor_ratio(portfolio_return: float, risk_free: float,
                  beta: float) -> float:
    """Eq. 68: RVOL_p = (ar_p - ar_f) / beta_p."""
    return (portfolio_return - risk_free) / beta


def sharpe_ratio(portfolio_return: float, risk_free: float,
                 std_dev: float) -> float:
    """Eq. 69: SR_p = (ar_p - ar_f) / sigma_p."""
    return (portfolio_return - risk_free) / std_dev


def m_squared(portfolio_return: float, risk_free: float, std_dev: float,
              market_std_dev: float) -> float:
    """Eq. 70: M^2 = ar_f + ((ar_p - ar_f) / sigma_p) sigma_M."""
    return risk_free + sharpe_ratio(portfolio_return, risk_free,
                                    std_dev) * market_std_dev


def quadratic_characteristic_curve(a: float, b: float, c: float,
                                   market_return: float,
                                   risk_free: float) -> float:
    """Eq. 71: r_p - r_f = a + b (r_M - r_f) + c (r_M - r_f)^2."""
    excess = market_return - risk_free
    return a + b * excess + c * excess**2


def dummy_characteristic_lines(a: float, b: float, c: float,
                               market_return: float,
                               risk_free: float) -> float:
    """Eq. 72: a + b (r_M - r_f) + c D (r_M - r_f).

    D = 0 when r_M > r_f and D = -1 when r_M < r_f.
    """
    excess = market_return - risk_free
    dummy = 0.0 if market_return > risk_free else -1.0
    return a + b * excess + c * dummy * excess


def foreign_return(local_return: float, currency_return: float) -> float:
    """Eq. 73: r_F = r_d + r_e + r_d r_e."""
    return local_return + currency_return + local_return * currency_return


def foreign_std_dev(local_std_dev: float, currency_std_dev: float,
                    correlation: float) -> float:
    """Eq. 74: sigma_F = (sigma_d^2 + sigma_e^2 + 2 rho sigma_d sigma_e)^(1/2)."""
    return math.sqrt(local_std_dev**2 + currency_std_dev**2 +
                     2.0 * correlation * local_std_dev * currency_std_dev)
