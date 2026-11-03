"""Two-factor models and the APT (equations 21 to 24)."""

__all__ = [
    "two_factor_return",
    "two_factor_variance",
    "two_factor_covariance",
    "apt_expected_return",
    "apt_expected_return_from_factor_portfolios",
]


def two_factor_return(a: float, b1: float, f1: float, b2: float, f2: float,
                      error: float = 0.0) -> float:
    """Eq. 21: r_i = a_i + b_i1 F_1 + b_i2 F_2 + e_i."""
    return a + b1 * f1 + b2 * f2 + error


def two_factor_variance(b1: float, b2: float, var_f1: float, var_f2: float,
                        cov_f1_f2: float, residual_variance: float) -> float:
    """Eq. 22: variance of a security under a two-factor model."""
    return (b1**2 * var_f1 + b2**2 * var_f2 + 2.0 * b1 * b2 * cov_f1_f2 +
            residual_variance)


def two_factor_covariance(b_i1: float, b_i2: float, b_j1: float, b_j2: float,
                          var_f1: float, var_f2: float,
                          cov_f1_f2: float) -> float:
    """Eq. 23: covariance of two securities under a two-factor model."""
    return (b_i1 * b_j1 * var_f1 + b_i2 * b_j2 * var_f2 +
            (b_i1 * b_j2 + b_i2 * b_j1) * cov_f1_f2)


def apt_expected_return(lambda0: float, lambda1: float, lambda2: float,
                        b1: float, b2: float) -> float:
    """Eq. 24 (first form): r_i = lambda_0 + lambda_1 b_i1 + lambda_2 b_i2."""
    return lambda0 + lambda1 * b1 + lambda2 * b2


def apt_expected_return_from_factor_portfolios(risk_free: float,
                                               delta1: float, delta2: float,
                                               b1: float, b2: float) -> float:
    """Eq. 24 (second form): r_f + (delta_1 - r_f) b_i1 + (delta_2 - r_f) b_i2."""
    return risk_free + (delta1 - risk_free) * b1 + (delta2 - risk_free) * b2
