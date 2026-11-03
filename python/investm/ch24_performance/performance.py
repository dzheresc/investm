"""Chapter 24: portfolio performance evaluation.

Mirrors cpp/ch24_performance/performance.h.
"""

import dataclasses
import math
from typing import Optional, Sequence

import numpy as np

from investm.ch05_riskless_securities import cash_flows as cf
from investm.ch07_portfolio_analysis import efficient_set as es
from investm.core import regression
from investm.core import stats


def time_weighted_return(period_returns: Sequence[float]) -> float:
    return math.prod(1 + r for r in period_returns) - 1


def period_returns_from_values(start: float, value_end: Sequence[float],
                               flows: Sequence[float]) -> list[float]:
    if len(value_end) != len(flows):
        raise ValueError("size mismatch")
    out = []
    begin = start
    for v, f in zip(value_end, flows):
        out.append(v / begin - 1)
        begin = v + f
    return out


def dollar_weighted_return(start: float, flows: Sequence[float],
                           end_value: float) -> Optional[float]:
    stream = [cf.CashFlow(0.0, -start)]
    stream += [cf.CashFlow(float(t + 1), -f) for t, f in enumerate(flows)]
    stream.append(cf.CashFlow(float(len(flows) + 1), end_value))
    return cf.internal_rate_of_return(stream)


@dataclasses.dataclass(frozen=True)
class PerformanceStats:
    """Risk-adjusted measures from per-period returns."""
    mean_excess: float
    std_dev: float
    beta: float
    alpha: float
    alpha_t_stat: float
    residual_std_dev: float
    sharpe: float
    treynor: float
    m_squared: float
    appraisal_ratio: float


def _excess(r: Sequence[float], rf: Sequence[float]) -> list[float]:
    if len(r) != len(rf):
        raise ValueError("size mismatch")
    return [a - b for a, b in zip(r, rf)]


def evaluate(portfolio: Sequence[float], market: Sequence[float],
             riskfree: Sequence[float]) -> PerformanceStats:
    ep = _excess(portfolio, riskfree)
    em = _excess(market, riskfree)
    fit = stats.ols(em, ep)
    mean_excess = stats.mean(ep)
    sd = stats.std_dev(portfolio)
    sd_m = stats.std_dev(market)
    sharpe = mean_excess / sd
    return PerformanceStats(
        mean_excess=mean_excess,
        std_dev=sd,
        beta=fit.beta,
        alpha=fit.alpha,
        alpha_t_stat=(fit.alpha /
                      fit.alpha_std_error if fit.alpha_std_error > 0 else 0.0),
        residual_std_dev=fit.residual_std_dev,
        sharpe=sharpe,
        treynor=mean_excess / fit.beta,
        m_squared=(sharpe - stats.mean(em) / sd_m) * sd_m,
        appraisal_ratio=(fit.alpha / fit.residual_std_dev
                         if fit.residual_std_dev > 0 else 0.0))


def information_ratio(portfolio: Sequence[float],
                      benchmark: Sequence[float]) -> float:
    active = _excess(portfolio, benchmark)
    return stats.mean(active) / stats.std_dev(active)


@dataclasses.dataclass(frozen=True)
class TimingFit:
    alpha: float
    beta: float
    timing: float
    timing_t_stat: float


def _timing(portfolio, market, riskfree,
            quadratic: bool) -> Optional[TimingFit]:
    ep = np.array(_excess(portfolio, riskfree))
    em = np.array(_excess(market, riskfree))
    extra = em * em if quadratic else np.maximum(0.0, -em)
    fit = regression.multiple_ols(np.column_stack([em, extra]), ep)
    if fit is None:
        return None
    c = fit.coefficients
    se = fit.std_errors[2]
    return TimingFit(float(c[0]), float(c[1]), float(c[2]),
                     float(c[2] / se) if se > 0 else 0.0)


def treynor_mazuy(portfolio, market, riskfree) -> Optional[TimingFit]:
    return _timing(portfolio, market, riskfree, True)


def henriksson_merton(portfolio, market, riskfree) -> Optional[TimingFit]:
    return _timing(portfolio, market, riskfree, False)


@dataclasses.dataclass(frozen=True)
class Attribution:
    allocation: np.ndarray
    selection: np.ndarray
    interaction: np.ndarray
    portfolio_return: float
    benchmark_return: float

    @property
    def total(self) -> float:
        return self.portfolio_return - self.benchmark_return


def brinson_attribution(wp, wb, rp, rb) -> Attribution:
    wp, wb, rp, rb = (np.asarray(x, dtype=float) for x in (wp, wb, rp, rb))
    total_b = float(wb @ rb)
    return Attribution((wp - wb) * (rb - total_b), wb * (rp - rb),
                       (wp - wb) * (rp - rb), float(wp @ rp), total_b)


@dataclasses.dataclass(frozen=True)
class StyleFit:
    weights: np.ndarray
    r_squared: float


def style_analysis(fund, styles) -> StyleFit:
    """Min var(fund - styles @ w) over the simplex (accelerated gradient)."""
    y = np.asarray(fund, dtype=float)
    x = np.asarray(styles, dtype=float)
    s = np.atleast_2d(np.cov(x, rowvar=False))
    c = np.array([np.cov(x[:, i], y)[0, 1] for i in range(x.shape[1])])
    step = 1.0 / (2.0 * np.linalg.eigvalsh(s)[-1])
    k = x.shape[1]
    w = np.full(k, 1.0 / k)
    z = w.copy()
    t = 1.0
    for _ in range(50000):
        nxt = es.project_onto_simplex(z - step * 2.0 * (s @ z - c))
        t_next = 0.5 * (1 + math.sqrt(1 + 4 * t * t))
        z = nxt + (t - 1) / t_next * (nxt - w)
        change = float(np.max(np.abs(nxt - w)))
        w, t = nxt, t_next
        if change < 1e-13:
            break
    resid = y - x @ w
    return StyleFit(w, 1 - float(np.var(resid, ddof=1) / np.var(y, ddof=1)))
