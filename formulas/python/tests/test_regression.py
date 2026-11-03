"""Tests for equations 38 to 43 against a direct least squares fit."""

import math
import statistics

import pytest

import investm_formulas as f

X = [0.02, -0.01, 0.03, 0.015, -0.02, 0.01, 0.025]
Y = [0.025, -0.02, 0.04, 0.01, -0.015, 0.02, 0.03]


def _reference():
    t = len(X)
    mx, my = statistics.fmean(X), statistics.fmean(Y)
    sxx = sum((x - mx)**2 for x in X)
    sxy = sum((x - mx) * (y - my) for x, y in zip(X, Y))
    beta = sxy / sxx
    alpha = my - beta * mx
    sse = sum((y - alpha - beta * x)**2 for x, y in zip(X, Y))
    se = math.sqrt(sse / (t - 2))
    return alpha, beta, se, se / math.sqrt(sxx), se * math.sqrt(
        sum(x * x for x in X) / (t * sxx))


def test_against_reference():
    alpha, beta, se, se_beta, se_alpha = _reference()
    b = f.regression_beta(X, Y)
    a = f.regression_alpha(X, Y, b)
    assert b == pytest.approx(beta)
    assert a == pytest.approx(alpha)
    s = f.regression_residual_std_dev(X, Y, a, b)
    assert s == pytest.approx(se)
    assert f.regression_beta_std_error(X, s) == pytest.approx(se_beta)
    assert f.regression_alpha_std_error(X, s) == pytest.approx(se_alpha)


def test_correlation():
    mx, my = statistics.fmean(X), statistics.fmean(Y)
    num = sum((x - mx) * (y - my) for x, y in zip(X, Y))
    den = math.sqrt(sum((x - mx)**2 for x in X) * sum((y - my)**2 for y in Y))
    assert f.correlation_coefficient(X, Y) == pytest.approx(num / den)
    assert f.correlation_coefficient(X, [2 * x + 1 for x in X]) == \
        pytest.approx(1.0)


def test_bad_input():
    with pytest.raises(ValueError):
        f.regression_beta([1.0, 2.0], [1.0])
