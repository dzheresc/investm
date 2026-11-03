"""Chapter 10: factor models and their estimation.

Mirrors cpp/ch10_factor_models/factor_models.h. A K-factor model
r = a + B F + e implies E[r] = a + B E[F] and Cov(r) = B S_F B' + D.
"""

import dataclasses
from typing import Optional, Sequence

import numpy as np

from investm.core import regression


@dataclasses.dataclass(frozen=True)
class FactorModel:
    intercepts: np.ndarray  # a, length N
    sensitivities: np.ndarray  # B, N x K
    residual_variances: np.ndarray  # length N


def factor_expected_returns(model: FactorModel, factor_means) -> np.ndarray:
    return model.intercepts + model.sensitivities @ np.asarray(
        factor_means, float)


def factor_covariance(model: FactorModel, factor_cov) -> np.ndarray:
    b = model.sensitivities
    return b @ np.asarray(factor_cov, float) @ b.T + np.diag(
        model.residual_variances)


def portfolio_sensitivities(model: FactorModel, weights) -> np.ndarray:
    return model.sensitivities.T @ np.asarray(weights, float)


def decompose_portfolio_variance(model: FactorModel, factor_cov,
                                 weights) -> tuple[float, float]:
    """(factor variance, residual variance) of a portfolio."""
    w = np.asarray(weights, float)
    b = portfolio_sensitivities(model, w)
    return (float(b @ np.asarray(factor_cov, float) @ b),
            float((w * w) @ model.residual_variances))


def estimate_time_series(returns, factors) -> Optional[FactorModel]:
    """10.4.1: regress each security (column of T x N) on T x K factors.

    Every security shares the regressors, so (X'X)^-1 X' is formed once and
    applied to all N columns in one product (mirrors OlsDesign in C++).
    """
    r = np.asarray(returns, float)
    f = np.asarray(factors, float)
    if f.ndim == 1:
        f = f[:, None]
    if r.shape[0] != f.shape[0]:
        raise ValueError("returns and factors cover different periods")
    design = np.column_stack([np.ones(r.shape[0]), f])
    n, k = design.shape
    if n <= k:
        return None
    xtx = design.T @ design
    if np.linalg.matrix_rank(xtx) < k:
        return None
    coef = np.linalg.inv(xtx) @ (design.T @ r)  # K x N.
    resid = r - design @ coef
    s2 = (resid * resid).sum(axis=0) / (n - k)
    return FactorModel(coef[0].copy(), coef[1:].T.copy(), s2)


def estimate_cross_sectional(
        returns,
        attributes,
        with_zero_factor: bool = True) -> Optional[np.ndarray]:
    """10.4.2: per-period regression of returns on attributes (N x K)."""
    r = np.asarray(returns, float)
    out = []
    for row in r:
        fit = regression.multiple_ols(attributes, row, with_zero_factor)
        if fit is None:
            return None
        out.append(fit.coefficients)
    return np.array(out)


@dataclasses.dataclass(frozen=True)
class PrincipalFactors:
    loadings: np.ndarray  # N x K
    eigenvalues: np.ndarray  # decreasing
    explained_fraction: float


def estimate_principal_components(returns,
                                  num_factors: int) -> PrincipalFactors:
    """10.4.3: principal components of the sample covariance matrix."""
    cov = np.cov(np.asarray(returns, float), rowvar=False)
    vals, vecs = np.linalg.eigh(cov)
    order = np.argsort(vals)[::-1]
    vals, vecs = vals[order], vecs[:, order]
    k = num_factors
    signs = np.where(vecs[:, :k].sum(axis=0) < 0, -1.0, 1.0)
    loadings = vecs[:, :k] * signs * np.sqrt(np.maximum(vals[:k], 0))
    return PrincipalFactors(loadings, vals, float(vals[:k].sum() / vals.sum()))


def sector_dummies(sector_of_security: Sequence[int],
                   num_sectors: int) -> np.ndarray:
    d = np.zeros((len(sector_of_security), num_sectors))
    for i, s in enumerate(sector_of_security):
        if not 0 <= s < num_sectors:
            raise ValueError("sector out of range")
        d[i, s] = 1.0
    return d
