"""Tests for equations 6 to 20."""

import math

import pytest

import investm_formulas as f

WEIGHTS = [0.5, 0.3, 0.2]
SD = [0.20, 0.15, 0.10]
RHO = [[1.0, 0.3, 0.1], [0.3, 1.0, 0.4], [0.1, 0.4, 1.0]]
COV = [[f.covariance(RHO[i][j], SD[i], SD[j]) for j in range(3)]
       for i in range(3)]


def test_expected_return():
    assert f.portfolio_expected_return(WEIGHTS, [0.1, 0.08, 0.05]) == \
        pytest.approx(0.05 + 0.024 + 0.01)


def test_length_mismatch_raises():
    with pytest.raises(ValueError):
        f.portfolio_expected_return([0.5, 0.5], [0.1])


def test_two_asset_matches_general():
    w = [0.6, 0.4]
    cov = [[f.covariance(1.0, 0.2, 0.2), f.covariance(0.25, 0.2, 0.3)],
           [f.covariance(0.25, 0.3, 0.2), f.covariance(1.0, 0.3, 0.3)]]
    assert f.two_asset_std_dev(0.6, 0.4, 0.2, 0.3, 0.25) == \
        pytest.approx(f.portfolio_std_dev(w, cov))


def test_portfolio_std_dev_perfect_correlation():
    cov = [[f.covariance(1.0, a, b) for b in SD] for a in SD]
    expected = sum(w * s for w, s in zip(WEIGHTS, SD))
    assert f.portfolio_std_dev(WEIGHTS, cov) == pytest.approx(expected)
    assert f.portfolio_variance(WEIGHTS, COV) < expected**2


def test_market_model():
    betas = [1.2, 0.8, 1.0]
    resid = [0.01, 0.02, 0.015]
    var_i = 0.04
    assert f.market_model_return(0.01, 1.2, 0.1, 0.0) == pytest.approx(0.13)
    assert f.market_model_beta(0.048, var_i) == pytest.approx(1.2)
    assert f.market_model_variance(1.2, var_i, 0.01) == \
        pytest.approx(1.44 * 0.04 + 0.01)
    assert f.market_model_covariance(1.2, 0.8, var_i) == \
        pytest.approx(0.0384)
    bp = f.portfolio_beta(WEIGHTS, betas)
    assert bp == pytest.approx(0.6 + 0.24 + 0.2)
    unique = f.portfolio_unique_variance(WEIGHTS, resid)
    assert unique == pytest.approx(0.25 * 0.01 + 0.09 * 0.02 + 0.04 * 0.015)
    # Eq. 14 must equal eq. 8 squared on the market-model covariance matrix.
    cov = [[f.market_model_covariance(bi, bj, var_i) +
            (resid[i] if i == j else 0.0)
            for j, bj in enumerate(betas)] for i, bi in enumerate(betas)]
    assert f.portfolio_variance_market_model(bp, var_i, unique) == \
        pytest.approx(f.portfolio_variance(WEIGHTS, cov))


def test_cml_and_sml():
    assert f.capital_market_line(0.04, 0.10, 0.2, 0.1) == pytest.approx(0.07)
    beta = f.capm_beta(0.048, 0.04)
    assert beta == pytest.approx(1.2)
    assert f.security_market_line(0.04, 0.10, beta) == pytest.approx(0.112)
    assert f.security_market_line_cov(0.04, 0.10, 0.04, 0.048) == \
        pytest.approx(0.112)


def test_market_variance_is_double_sum():
    assert f.portfolio_variance(WEIGHTS, COV) == pytest.approx(
        math.fsum(WEIGHTS[i] * WEIGHTS[j] * COV[i][j]
                  for i in range(3) for j in range(3)))
