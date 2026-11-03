"""Tests for equations 35 to 37 and 44 to 54."""

import pytest

import investm_formulas as f


def test_voting_and_rights():
    assert f.shares_to_elect_directors(2, 1000, 4) == pytest.approx(401.0)
    assert f.right_value_rights_on(60.0, 50.0, 4) == pytest.approx(2.0)
    assert f.right_value_ex_rights(58.0, 50.0, 4) == pytest.approx(2.0)


def test_firm_and_equity_beta_round_trip():
    tax, debt, equity, beta_debt, beta_firm = 0.3, 40.0, 60.0, 0.2, 0.9
    unlevered = equity + (1.0 - tax) * debt
    beta_eq = f.equity_beta(beta_firm, beta_debt, debt, equity, tax)
    assert f.firm_beta(beta_debt, beta_eq, tax, debt, equity, unlevered) == \
        pytest.approx(beta_firm)


def test_adjusted_beta():
    assert f.adjusted_beta(0.33, 0.67, 1.5) == pytest.approx(1.335)


def test_ddm():
    assert f.zero_growth_ddm(2.0, 0.1) == pytest.approx(20.0)
    assert f.constant_growth_ddm(2.0, 0.1, 0.05) == pytest.approx(40.0)
    with pytest.raises(ValueError):
        f.constant_growth_ddm(2.0, 0.05, 0.05)
    # With T = 0 the multiple-growth model reduces to constant growth.
    assert f.multiple_growth_ddm([], 2.0, 0.1, 0.05) == pytest.approx(40.0)
    # Two explicit dividends, then constant growth from D_3.
    v = f.multiple_growth_ddm([1.0, 1.5], 1.575, 0.1, 0.05)
    assert v == pytest.approx(1 / 1.1 + 1.5 / 1.21 + 1.575 / (0.05 * 1.21))
    assert f.capitalized_value([1.0, 1.0], 0.1) == \
        pytest.approx(1 / 1.1 + 1 / 1.21)


def test_normal_pe():
    # Full payout: P/E = 1/k.
    assert f.normal_price_earnings(1.0, 0.15, 0.1) == pytest.approx(10.0)
    g = 0.15 * 0.4
    assert f.normal_price_earnings(0.6, 0.15, 0.1) == \
        pytest.approx(0.6 * (1 + g) / (0.1 - g))


def test_earnings_models():
    assert f.lintner_dividend(0.5, 0.4, 10.0, 3.0) == pytest.approx(3.5)
    assert f.random_walk_earnings(5.0, -0.5) == pytest.approx(4.5)
    assert f.quarterly_earnings(1.0, 1.2, 0.9, 0.4, 0.05, 0.01) == \
        pytest.approx(1.0 + 0.12 + 0.05 + 0.01)
