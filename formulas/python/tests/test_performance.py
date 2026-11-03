"""Tests for equations 61 to 74."""

import pytest

import investm_formulas as f


def test_investment_company():
    assert f.net_asset_value(1_000_000.0, 50_000.0, 100_000.0) == \
        pytest.approx(9.5)
    assert f.investment_company_return(10.5, 10.0, 0.2, 0.3) == \
        pytest.approx(0.1)


def test_risk_tolerance_and_certainty_equivalent():
    tau = f.implied_risk_tolerance(0.08, 0.12, 0.04, 0.2)
    assert tau == pytest.approx(2 * 0.04 * 0.04 / 0.0064)
    assert f.certainty_equivalent_return(0.10, 0.2, 0.5) == \
        pytest.approx(0.10 - 0.08)


def test_alphas_and_characteristic_line():
    assert f.ex_post_alpha(0.12, 0.10) == pytest.approx(0.02)
    alpha = f.capm_ex_post_alpha(0.15, 0.04, 0.10, 1.2)
    assert alpha == pytest.approx(0.15 - 0.112)
    excess = f.characteristic_line(alpha, 1.2, 0.10, 0.04)
    assert excess + 0.04 == pytest.approx(0.15)


def test_ratios():
    assert f.treynor_ratio(0.12, 0.04, 0.8) == pytest.approx(0.1)
    assert f.sharpe_ratio(0.12, 0.04, 0.2) == pytest.approx(0.4)
    assert f.m_squared(0.12, 0.04, 0.2, 0.15) == pytest.approx(0.1)


def test_characteristic_curves():
    assert f.quadratic_characteristic_curve(0.01, 1.0, 2.0, 0.14, 0.04) == \
        pytest.approx(0.01 + 0.1 + 0.02)
    up = f.dummy_characteristic_lines(0.01, 1.0, 0.5, 0.14, 0.04)
    down = f.dummy_characteristic_lines(0.01, 1.0, 0.5, -0.06, 0.04)
    assert up == pytest.approx(0.11)
    assert down == pytest.approx(0.01 - 0.1 + 0.05)


def test_foreign():
    assert f.foreign_return(0.10, -0.05) == pytest.approx(1.10 * 0.95 - 1.0)
    assert f.foreign_std_dev(0.2, 0.1, 1.0) == pytest.approx(0.3)
    assert f.foreign_std_dev(0.2, 0.1, 0.0) == pytest.approx(0.05**0.5)
