"""Tests for equations 55 to 60."""

import math

import pytest

import investm_formulas as f


def test_intrinsic_values():
    assert f.call_intrinsic_value(110.0, 100.0) == 10.0
    assert f.call_intrinsic_value(90.0, 100.0) == 0.0
    assert f.put_intrinsic_value(90.0, 100.0) == 10.0
    assert f.put_intrinsic_value(110.0, 100.0) == 0.0


def test_black_scholes_reference_values():
    # Textbook case: S = K = 100, r = 5%, T = 1, sigma = 20%.
    assert f.black_scholes_call(100, 100, 0.05, 1.0, 0.2) == \
        pytest.approx(10.450584, abs=1e-5)
    assert f.black_scholes_put(100, 100, 0.05, 1.0, 0.2) == \
        pytest.approx(5.573526, abs=1e-5)
    d1 = f.black_scholes_d1(100, 100, 0.05, 1.0, 0.2)
    assert d1 == pytest.approx(0.35)
    assert f.black_scholes_d2(100, 100, 0.05, 1.0, 0.2) == pytest.approx(0.15)


def test_black_scholes_obeys_parity():
    s, k, r, t, v = 50.0, 55.0, 0.03, 0.5, 0.3
    call = f.black_scholes_call(s, k, r, t, v)
    put = f.black_scholes_put(s, k, r, t, v)
    assert f.put_from_parity(call, s, k, r, t) == pytest.approx(put)
    assert f.call_from_parity(put, s, k, r, t) == pytest.approx(call)
    assert put + s == pytest.approx(call + k * math.exp(-r * t))


def test_futures():
    assert f.futures_fair_value(100.0, 5.0, 2.0, 1.0) == pytest.approx(104.0)
    assert f.index_futures_fair_value(1000.0, 0.05, 0.02) == \
        pytest.approx(1030.0)
