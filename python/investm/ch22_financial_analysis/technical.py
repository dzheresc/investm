"""Chapter 22: technical analysis.

Mirrors cpp/ch22_financial_analysis/technical.h.
"""

import math
from typing import Sequence


def simple_moving_average(prices: Sequence[float], n: int) -> list[float]:
    if n < 1:
        raise ValueError("window must be at least 1")
    out = [math.nan] * len(prices)
    total = 0.0
    for t, p in enumerate(prices):
        total += p
        if t >= n:
            total -= prices[t - n]
        if t + 1 >= n:
            out[t] = total / n
    return out


def moving_average_signal(prices: Sequence[float], n: int) -> list[int]:
    ma = simple_moving_average(prices, n)
    signal = []
    for p, m in zip(prices, ma):
        if math.isnan(m) or p == m:
            signal.append(0)
        else:
            signal.append(1 if p > m else -1)
    return signal


def momentum(prices: Sequence[float], lag: int) -> list[float]:
    return [
        math.nan if t < lag else prices[t] / prices[t - lag] - 1
        for t in range(len(prices))
    ]


def relative_strength(stock: Sequence[float],
                      index: Sequence[float]) -> list[float]:
    if len(stock) != len(index) or len(stock) == 0:
        raise ValueError("size mismatch")
    base = stock[0] / index[0]
    return [s / i / base for s, i in zip(stock, index)]


def moving_average_rule_returns(prices: Sequence[float], n: int,
                                cash_rate: float) -> list[float]:
    signal = moving_average_signal(prices, n)
    return [
        prices[t + 1] / prices[t] - 1 if signal[t] == 1 else cash_rate
        for t in range(len(prices) - 1)
    ]
