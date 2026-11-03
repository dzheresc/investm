"""Portfolio theory, the market model and the CAPM (equations 6 to 20)."""

import math
from typing import Sequence

__all__ = [
    "portfolio_expected_return",
    "covariance",
    "portfolio_variance",
    "portfolio_std_dev",
    "two_asset_std_dev",
    "market_model_return",
    "market_model_beta",
    "market_model_variance",
    "market_model_covariance",
    "portfolio_variance_market_model",
    "portfolio_beta",
    "portfolio_unique_variance",
    "capital_market_line",
    "security_market_line_cov",
    "security_market_line",
    "capm_beta",
]


def _check_same_length(a: Sequence[float], b: Sequence[float]) -> None:
    if len(a) != len(b):
        raise ValueError(f"length mismatch: {len(a)} != {len(b)}")


def _weighted_sum(weights: Sequence[float], values: Sequence[float]) -> float:
    _check_same_length(weights, values)
    return math.fsum(w * v for w, v in zip(weights, values))


def portfolio_expected_return(weights: Sequence[float],
                              expected_returns: Sequence[float]) -> float:
    """Eq. 6: sum of X_i * r_i."""
    return _weighted_sum(weights, expected_returns)


def covariance(correlation: float, std_dev_i: float, std_dev_j: float) -> float:
    """Eq. 7: sigma_ij = rho_ij * sigma_i * sigma_j."""
    return correlation * std_dev_i * std_dev_j


def portfolio_variance(weights: Sequence[float],
                       cov: Sequence[Sequence[float]]) -> float:
    """Eq. 8 squared and eq. 18: sum_i sum_j X_i X_j sigma_ij."""
    if len(cov) != len(weights):
        raise ValueError("covariance matrix must be N x N")
    total = []
    for x_i, row in zip(weights, cov):
        _check_same_length(weights, row)
        total.extend(x_i * x_j * s for x_j, s in zip(weights, row))
    return math.fsum(total)


def portfolio_std_dev(weights: Sequence[float],
                      cov: Sequence[Sequence[float]]) -> float:
    """Eq. 8: square root of the double sum of X_i X_j sigma_ij."""
    return math.sqrt(portfolio_variance(weights, cov))


def two_asset_std_dev(weight_1: float, weight_2: float, std_dev_1: float,
                      std_dev_2: float, correlation: float) -> float:
    """Eq. 9: standard deviation of a two-asset portfolio."""
    return math.sqrt(weight_1**2 * std_dev_1**2 + weight_2**2 * std_dev_2**2 +
                     2.0 * weight_1 * weight_2 * correlation * std_dev_1 *
                     std_dev_2)


def market_model_return(alpha: float, beta: float, index_return: float,
                        error: float = 0.0) -> float:
    """Eq. 10: r_i = alpha_i + beta_i * r_I + epsilon_i."""
    return alpha + beta * index_return + error


def market_model_beta(cov_with_index: float, index_variance: float) -> float:
    """Eq. 11: beta_i = sigma_iI / sigma_I^2."""
    return cov_with_index / index_variance


def market_model_variance(beta: float, index_variance: float,
                          residual_variance: float) -> float:
    """Eq. 12: sigma_i^2 = beta_i^2 sigma_I^2 + sigma_ei^2."""
    return beta**2 * index_variance + residual_variance


def market_model_covariance(beta_i: float, beta_j: float,
                            index_variance: float) -> float:
    """Eq. 13: sigma_ij = beta_i beta_j sigma_I^2."""
    return beta_i * beta_j * index_variance


def portfolio_variance_market_model(portfolio_beta_value: float,
                                    index_variance: float,
                                    unique_variance: float) -> float:
    """Eq. 14: sigma_p^2 = beta_p^2 sigma_I^2 + sigma_ep^2."""
    return portfolio_beta_value**2 * index_variance + unique_variance


def portfolio_beta(weights: Sequence[float], betas: Sequence[float]) -> float:
    """Eq. 15: beta_p = sum of X_i beta_i."""
    return _weighted_sum(weights, betas)


def portfolio_unique_variance(weights: Sequence[float],
                              residual_variances: Sequence[float]) -> float:
    """Eq. 16: sigma_ep^2 = sum of X_i^2 sigma_ei^2."""
    _check_same_length(weights, residual_variances)
    return math.fsum(w * w * v for w, v in zip(weights, residual_variances))


def capital_market_line(risk_free: float, market_return: float,
                        market_std_dev: float,
                        portfolio_std_dev_value: float) -> float:
    """Eq. 17: r_p = r_f + [(r_M - r_f) / sigma_M] sigma_p."""
    return risk_free + (market_return - risk_free) / market_std_dev * \
        portfolio_std_dev_value


def security_market_line_cov(risk_free: float, market_return: float,
                             market_variance: float,
                             cov_with_market: float) -> float:
    """Eq. 19 (covariance version): r_f + [(r_M - r_f)/sigma_M^2] sigma_iM."""
    return risk_free + (market_return - risk_free) / market_variance * \
        cov_with_market


def security_market_line(risk_free: float, market_return: float,
                         beta: float) -> float:
    """Eq. 19 (beta version): r_i = r_f + (r_M - r_f) beta_i."""
    return risk_free + (market_return - risk_free) * beta


def capm_beta(cov_with_market: float, market_variance: float) -> float:
    """Eq. 20: beta_iM = sigma_iM / sigma_M^2."""
    return cov_with_market / market_variance
