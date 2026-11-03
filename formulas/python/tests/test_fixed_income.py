"""Tests for equations 25 to 34."""

import pytest

import investm_formulas as f


def test_tax_growth_real():
    assert f.break_even_tax_rate(0.06, 0.08) == pytest.approx(0.25)
    assert f.geometric_mean_growth_rate(100.0, 121.0, 2) == pytest.approx(0.1)
    assert f.real_return(0.10, 0.04) == pytest.approx(1.10 / 1.04 - 1.0)
    assert f.approx_real_return(0.10, 0.04) == pytest.approx(0.06)


def test_par_bond_and_ytm():
    coupons = [5.0] * 10
    assert f.bond_price(coupons, 100.0, 0.05) == pytest.approx(100.0)
    price = f.bond_price(coupons, 100.0, 0.07)
    assert f.yield_to_maturity(price, coupons, 100.0) == \
        pytest.approx(0.07, abs=1e-9)


def test_spot_and_forward():
    assert f.discount_factor(0.05, 2) == pytest.approx(1 / 1.05**2)
    fwd = f.forward_rate(0.05, 0.06, 2)
    assert fwd == pytest.approx(1.06**2 / 1.05 - 1.0)
    assert (1.05) * (1.0 + fwd) == pytest.approx(1.06**2)


def test_duration():
    # A zero-coupon bond's duration is its maturity.
    assert f.macaulay_duration([0.0, 0.0, 100.0], 0.08) == pytest.approx(3.0)
    # Two-year 10% annual bond at 10%: (10/1.1 + 2*110/1.21) / 100.
    d = f.macaulay_duration([10.0, 110.0], 0.10)
    assert d == pytest.approx((10 / 1.1 + 2 * 110 / 1.21) / 100.0)
    dm = f.modified_duration(d, 0.10)
    assert dm == pytest.approx(d / 1.1)
    assert f.price_change_from_duration(d, 0.10, 0.001) == \
        pytest.approx(f.price_change_from_modified_duration(dm, 0.001))


def test_effective_duration_close_to_modified():
    coupons = [6.0] * 10
    y, dy = 0.06, 1e-4
    p = f.bond_price(coupons, 100.0, y)
    up = f.bond_price(coupons, 100.0, y + dy)
    down = f.bond_price(coupons, 100.0, y - dy)
    cfs = coupons[:-1] + [106.0]
    dm = f.modified_duration(f.macaulay_duration(cfs, y), y)
    assert f.effective_duration(down, up, p, dy) == pytest.approx(dm, rel=1e-6)
