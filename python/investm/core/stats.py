"""Descriptive statistics and simple regression (mirrors cpp/core/stats.h)."""

import dataclasses
import math
import operator
from typing import Sequence


def _check_same_size(x: Sequence[float], y: Sequence[float]) -> None:
    if len(x) != len(y):
        raise ValueError("size mismatch")


def mean(x: Sequence[float]) -> float:
    if len(x) == 0:
        raise ValueError("mean of empty sequence")
    return math.fsum(x) / len(x)


def covariance(x: Sequence[float], y: Sequence[float], ddof: int = 1) -> float:
    """Sample (ddof=1) or population (ddof=0) covariance."""
    _check_same_size(x, y)
    n = len(x)
    if n - ddof <= 0:
        raise ValueError("too few observations")
    mx, my = mean(x), mean(y)
    dx = [a - mx for a in x]
    dy = [b - my for b in y]
    return math.fsum(map(operator.mul, dx, dy)) / (n - ddof)


def variance(x: Sequence[float], ddof: int = 1) -> float:
    n = len(x)
    if n - ddof <= 0:
        raise ValueError("too few observations")
    m = mean(x)
    d = [a - m for a in x]
    return math.fsum(map(operator.mul, d, d)) / (n - ddof)


def std_dev(x: Sequence[float], ddof: int = 1) -> float:
    return math.sqrt(variance(x, ddof))


def correlation(x: Sequence[float], y: Sequence[float]) -> float:
    return covariance(x, y) / (std_dev(x) * std_dev(y))


def expected(outcomes: Sequence[float], probs: Sequence[float]) -> float:
    """Probability-weighted mean of a discrete distribution."""
    _check_same_size(outcomes, probs)
    return math.fsum(p * o for o, p in zip(outcomes, probs))


def expected_covariance(x: Sequence[float], y: Sequence[float],
                        probs: Sequence[float]) -> float:
    """Probability-weighted covariance of two discrete random variables."""
    _check_same_size(x, probs)
    _check_same_size(y, probs)
    mx, my = expected(x, probs), expected(y, probs)
    return math.fsum(p * (a - mx) * (b - my) for a, b, p in zip(x, y, probs))


def autocorrelation(x: Sequence[float], lag: int) -> float:
    """Lag-k sample autocorrelation."""
    if lag >= len(x):
        raise ValueError("lag too large")
    x = [float(v) for v in x]  # Plain floats: NumPy scalars are slow here.
    m = mean(x)
    d = [v - m for v in x]
    den = math.fsum(map(operator.mul, d, d))
    num = math.fsum(map(operator.mul, d[lag:], d[:len(d) - lag]))
    return num / den


def geometric_mean_return(returns: Sequence[float]) -> float:
    """Compound per-period return: (prod(1 + r))^(1/T) - 1."""
    if len(returns) == 0:
        raise ValueError("empty return series")
    if any(r <= -1.0 for r in returns):
        return -1.0
    return math.expm1(math.fsum(math.log1p(r) for r in returns) / len(returns))


@dataclasses.dataclass(frozen=True)
class SimpleRegression:
    """Result of an OLS fit y = alpha + beta * x + e."""
    alpha: float
    beta: float
    r_squared: float
    residual_std_dev: float
    alpha_std_error: float
    beta_std_error: float
    n: int

    def predict(self, x: float) -> float:
        return self.alpha + self.beta * x


def ols(x: Sequence[float], y: Sequence[float]) -> SimpleRegression:
    """Ordinary least squares with one regressor and an intercept."""
    _check_same_size(x, y)
    n = len(x)
    if n < 3:
        raise ValueError("OLS needs at least 3 points")
    mx, my = mean(x), mean(y)
    sxx = math.fsum((a - mx)**2 for a in x)
    sxy = math.fsum((a - mx) * (b - my) for a, b in zip(x, y))
    syy = math.fsum((b - my)**2 for b in y)
    if sxx == 0.0:
        raise ValueError("regressor has zero variance")
    beta = sxy / sxx
    alpha = my - beta * mx
    sse = math.fsum((b - alpha - beta * a)**2 for a, b in zip(x, y))
    s = math.sqrt(sse / (n - 2))
    return SimpleRegression(alpha=alpha,
                            beta=beta,
                            r_squared=1.0 if syy == 0.0 else 1.0 - sse / syy,
                            residual_std_dev=s,
                            alpha_std_error=s *
                            math.sqrt(1.0 / n + mx * mx / sxx),
                            beta_std_error=s / math.sqrt(sxx),
                            n=n)


def norm_pdf(x: float) -> float:
    return math.exp(-0.5 * x * x) / math.sqrt(2.0 * math.pi)


def norm_cdf(x: float) -> float:
    return 0.5 * math.erfc(-x / math.sqrt(2.0))
