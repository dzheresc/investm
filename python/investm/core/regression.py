"""Multiple linear regression by OLS (mirrors cpp/core/regression.h)."""

import dataclasses
from typing import Optional

import numpy as np


@dataclasses.dataclass(frozen=True)
class LinearRegression:
    """OLS fit; coefficients start with the intercept when there is one."""
    coefficients: np.ndarray
    std_errors: np.ndarray
    residuals: np.ndarray
    r_squared: float
    residual_std_dev: float
    has_intercept: bool

    def predict(self, x: np.ndarray) -> float:
        x = np.asarray(x, dtype=float)
        if self.has_intercept:
            return float(self.coefficients[0] + self.coefficients[1:] @ x)
        return float(self.coefficients @ x)


def multiple_ols(x: np.ndarray,
                 y: np.ndarray,
                 intercept: bool = True) -> Optional[LinearRegression]:
    """Regresses y on the columns of x.

    Args:
        x: T x K regressors (without a column of ones).
        y: Length-T dependent variable.
        intercept: Whether to fit an intercept.

    Returns:
        The fit, or None if X'X is singular or T <= number of coefficients.
    """
    x = np.asarray(x, dtype=float)
    y = np.asarray(y, dtype=float)
    if x.ndim == 1:
        x = x[:, None]
    if x.shape[0] != y.shape[0]:
        raise ValueError("X and y have different lengths")
    design = np.column_stack([np.ones(len(y)), x]) if intercept else x
    n, k = design.shape
    if n <= k:
        return None
    xtx = design.T @ design
    if np.linalg.matrix_rank(xtx) < k:
        return None
    xtx_inv = np.linalg.inv(xtx)
    coef = xtx_inv @ design.T @ y
    resid = y - design @ coef
    sse = float(resid @ resid)
    sst = float(((y - y.mean())**2).sum()) if intercept else float(y @ y)
    s2 = sse / (n - k)
    return LinearRegression(coefficients=coef,
                            std_errors=np.sqrt(s2 * np.diag(xtx_inv)),
                            residuals=resid,
                            r_squared=1.0 - sse / sst if sst > 0 else 1.0,
                            residual_std_dev=float(np.sqrt(s2)),
                            has_intercept=intercept)


def covariance_from_correlation(std_devs, corr) -> np.ndarray:
    """Builds a covariance matrix from standard deviations and correlations."""
    s = np.asarray(std_devs, dtype=float)
    return np.outer(s, s) * np.asarray(corr, dtype=float)
