"""Simple linear regression of Y on X (equations 38 to 43)."""

import math
from typing import NamedTuple, Sequence

__all__ = [
    "regression_beta",
    "regression_alpha",
    "regression_residual_std_dev",
    "regression_beta_std_error",
    "regression_alpha_std_error",
    "correlation_coefficient",
]


class _Sums(NamedTuple):
    t: int
    x: float
    y: float
    xx: float
    yy: float
    xy: float


def _sums(x: Sequence[float], y: Sequence[float]) -> _Sums:
    if len(x) != len(y):
        raise ValueError(f"length mismatch: {len(x)} != {len(y)}")
    if len(x) < 2:
        raise ValueError("need at least two observations")
    return _Sums(len(x), math.fsum(x), math.fsum(y),
                 math.fsum(v * v for v in x), math.fsum(v * v for v in y),
                 math.fsum(a * b for a, b in zip(x, y)))


def regression_beta(x: Sequence[float], y: Sequence[float]) -> float:
    """Eq. 38: (T sum XY - sum Y sum X) / (T sum X^2 - (sum X)^2)."""
    s = _sums(x, y)
    return (s.t * s.xy - s.y * s.x) / (s.t * s.xx - s.x**2)


def regression_alpha(x: Sequence[float], y: Sequence[float],
                     beta: float) -> float:
    """Eq. 39: sum Y / T - beta sum X / T."""
    s = _sums(x, y)
    return s.y / s.t - beta * s.x / s.t


def regression_residual_std_dev(x: Sequence[float], y: Sequence[float],
                                alpha: float, beta: float) -> float:
    """Eq. 40: [(sum Y^2 - alpha sum Y - beta sum XY) / (T - 2)]^(1/2)."""
    s = _sums(x, y)
    if s.t < 3:
        raise ValueError("need at least three observations")
    sse = s.yy - alpha * s.y - beta * s.xy
    return math.sqrt(max(sse, 0.0) / (s.t - 2))


def regression_beta_std_error(x: Sequence[float],
                              residual_std_dev: float) -> float:
    """Eq. 41: sigma_e / [sum X^2 - (sum X)^2 / T]^(1/2)."""
    t = len(x)
    sx = math.fsum(x)
    sxx = math.fsum(v * v for v in x)
    return residual_std_dev / math.sqrt(sxx - sx**2 / t)


def regression_alpha_std_error(x: Sequence[float],
                               residual_std_dev: float) -> float:
    """Eq. 42: sigma_e / [T - (sum X)^2 / sum X^2]^(1/2)."""
    t = len(x)
    sx = math.fsum(x)
    sxx = math.fsum(v * v for v in x)
    return residual_std_dev / math.sqrt(t - sx**2 / sxx)


def correlation_coefficient(x: Sequence[float], y: Sequence[float]) -> float:
    """Eq. 43: sample correlation in the sums form."""
    s = _sums(x, y)
    num = s.t * s.xy - s.y * s.x
    den = math.sqrt((s.t * s.yy - s.y**2) * (s.t * s.xx - s.x**2))
    return num / den
