"""One-dimensional root finding.

Yields, internal rates of return and implied volatilities are all "find x
such that price(x) equals the observed price". Mirrors cpp/core/root_find.h:
the solvers return None, rather than raising, when no root can be found.
"""

import math
from typing import Callable, Optional

Func = Callable[[float], float]


def bisect(f: Func,
           lo: float,
           hi: float,
           tol: float = 1e-12,
           max_iter: int = 200) -> Optional[float]:
    """Finds a root of f in [lo, hi] by bisection.

    Args:
        f: Continuous function.
        lo: Lower end of the bracket.
        hi: Upper end of the bracket.
        tol: Absolute tolerance on x.
        max_iter: Iteration cap.

    Returns:
        The root, or None if f(lo) and f(hi) have the same sign.
    """
    f_lo, f_hi = f(lo), f(hi)
    if f_lo == 0.0:
        return lo
    if f_hi == 0.0:
        return hi
    if (f_lo > 0.0) == (f_hi > 0.0):
        return None
    for _ in range(max_iter):
        if hi - lo <= tol:
            break
        mid = 0.5 * (lo + hi)
        f_mid = f(mid)
        if f_mid == 0.0:
            return mid
        if (f_mid > 0.0) == (f_lo > 0.0):
            lo, f_lo = mid, f_mid
        else:
            hi = mid
    return 0.5 * (lo + hi)


def brent(f: Func,
          a: float,
          b: float,
          tol: float = 1e-12,
          max_iter: int = 200) -> Optional[float]:
    """Brent's method; the default solver. Same contract as bisect()."""
    f_a, f_b = f(a), f(b)
    if f_a == 0.0:
        return a
    if f_b == 0.0:
        return b
    if (f_a > 0.0) == (f_b > 0.0):
        return None
    if abs(f_a) < abs(f_b):
        a, b, f_a, f_b = b, a, f_b, f_a
    c, f_c, d = a, f_a, b - a
    bisected = True
    for _ in range(max_iter):
        if f_b == 0.0 or abs(b - a) < tol:
            return b
        if f_c not in (f_a, f_b):
            s = (a * f_b * f_c / ((f_a - f_b) * (f_a - f_c)) + b * f_a * f_c /
                 ((f_b - f_a) * (f_b - f_c)) + c * f_a * f_b / ((f_c - f_a) *
                                                                (f_c - f_b)))
        else:
            s = b - f_b * (b - a) / (f_b - f_a)
        quarter = (3.0 * a + b) / 4.0
        outside = not min(quarter, b) < s < max(quarter, b)
        if bisected:
            slow = abs(s - b) >= abs(b - c) / 2.0
            tiny = abs(b - c) < tol
        else:
            slow = abs(s - b) >= abs(c - d) / 2.0
            tiny = abs(c - d) < tol
        if outside or slow or tiny:
            s = 0.5 * (a + b)
            bisected = True
        else:
            bisected = False
        f_s = f(s)
        d, c, f_c = c, b, f_b
        if (f_a > 0.0) != (f_s > 0.0):
            b, f_b = s, f_s
        else:
            a, f_a = s, f_s
        if abs(f_a) < abs(f_b):
            a, b, f_a, f_b = b, a, f_b, f_a
    return b


def newton(f: Func,
           df: Func,
           x0: float,
           tol: float = 1e-12,
           max_iter: int = 200) -> Optional[float]:
    """Newton-Raphson with analytic derivative; None if it fails to converge."""
    x = x0
    for _ in range(max_iter):
        slope = df(x)
        if slope == 0.0 or not math.isfinite(slope):
            return None
        step = f(x) / slope
        x -= step
        if abs(step) < tol:
            return x
    return None


def brent_expand_up(f: Func,
                    lo: float,
                    hi: float,
                    tol: float = 1e-12) -> Optional[float]:
    """Doubles [lo, hi] upward until f changes sign, then runs brent()."""
    f_lo = f(lo)
    for _ in range(60):
        f_hi = f(hi)
        if f_hi == 0.0 or (f_lo > 0.0) != (f_hi > 0.0):
            return brent(f, lo, hi, tol)
        hi = lo + 2.0 * (hi - lo)
    return None
