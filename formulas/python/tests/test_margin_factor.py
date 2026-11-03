"""Tests for equations 1 to 5 and 21 to 24."""

import pytest

import investm_formulas as f


def test_rate_of_return():
    assert f.rate_of_return(100.0, 110.0) == pytest.approx(0.10)


def test_margin_purchase():
    # 100 shares bought at $100 with 60% initial margin, now at $80.
    assert f.actual_margin_purchase(100, 80.0, 0.6, 100.0) == pytest.approx(0.5)
    call = f.margin_call_price_purchase(0.6, 100.0, 0.3)
    assert call == pytest.approx(40.0 / 0.7)
    # At the call price the actual margin equals the maintenance margin.
    assert f.actual_margin_purchase(100, call, 0.6, 100.0) == pytest.approx(0.3)


def test_margin_short_sale():
    assert f.actual_margin_short_sale(100, 100.0, 0.5, 120.0) == \
        pytest.approx(0.25)
    call = f.margin_call_price_short_sale(100.0, 0.5, 0.3)
    assert call == pytest.approx(150.0 / 1.3)
    assert f.actual_margin_short_sale(100, 100.0, 0.5, call) == \
        pytest.approx(0.3)


def test_two_factor_model():
    assert f.two_factor_return(0.01, 1.2, 0.05, 0.5, 0.02, 0.003) == \
        pytest.approx(0.01 + 0.06 + 0.01 + 0.003)
    assert f.two_factor_variance(1.0, 2.0, 0.04, 0.01, 0.005, 0.02) == \
        pytest.approx(0.04 + 0.04 + 0.02 + 0.02)
    # Covariance of a security with itself, less residual, equals eq. 22.
    cov = f.two_factor_covariance(1.0, 2.0, 1.0, 2.0, 0.04, 0.01, 0.005)
    assert cov == pytest.approx(
        f.two_factor_variance(1.0, 2.0, 0.04, 0.01, 0.005, 0.0))


def test_apt_forms_agree():
    rf, d1, d2, b1, b2 = 0.04, 0.10, 0.07, 1.1, 0.6
    first = f.apt_expected_return(rf, d1 - rf, d2 - rf, b1, b2)
    second = f.apt_expected_return_from_factor_portfolios(rf, d1, d2, b1, b2)
    assert first == pytest.approx(second)
