"""Chapter 11: arbitrage portfolios and the APT pricing equation.

Mirrors cpp/ch11_apt/apt.h:
E[r_i] = lambda_0 + lambda_1 b_i1 + ... + lambda_K b_iK.
"""

import dataclasses
from typing import Optional

import numpy as np

from investm.core import regression


def _with_ones(b) -> np.ndarray:
    b = np.asarray(b, float)
    if b.ndim == 1:
        b = b[:, None]
    return np.column_stack([np.ones(b.shape[0]), b])


def is_arbitrage_portfolio(x, b, mu, tol: float = 1e-9) -> bool:
    """sum X = 0, B'X = 0 and mu'X > 0."""
    x = np.asarray(x, float)
    a = _with_ones(b)
    return bool(
        np.all(np.abs(a.T @ x) <= tol) and
        float(x @ np.asarray(mu, float)) > tol)


def find_arbitrage_portfolio(mu,
                             b,
                             scale: float = 1.0,
                             tol: float = 1e-10) -> Optional[np.ndarray]:
    """Residual of mu after projecting on [1, B]; None if mu is priced."""
    mu = np.asarray(mu, float)
    a = _with_ones(b)
    coef = np.linalg.lstsq(a, mu, rcond=None)[0]
    x = mu - a @ coef
    purchases = x[x > 0].sum()
    if purchases <= tol:
        return None
    return x * scale / purchases


@dataclasses.dataclass(frozen=True)
class AptPricing:
    lambda0: float
    lambdas: np.ndarray
    pricing_errors: np.ndarray


def fit_apt_pricing(mu, b) -> Optional[AptPricing]:
    fit = regression.multiple_ols(b, mu)
    if fit is None:
        return None
    return AptPricing(float(fit.coefficients[0]), fit.coefficients[1:],
                      fit.residuals)


def apt_expected_return(lambda0: float, lambdas, sensitivities) -> float:
    return lambda0 + float(
        np.asarray(lambdas, float) @ np.asarray(sensitivities, float))


def pure_factor_portfolio(b, k: int) -> np.ndarray:
    """Minimum-norm fully invested portfolio with b_p = e_k."""
    a = _with_ones(b)
    target = np.zeros(a.shape[1])
    target[0] = 1.0
    target[k + 1] = 1.0
    return a @ np.linalg.solve(a.T @ a, target)
