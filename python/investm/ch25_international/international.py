"""Chapter 25: international investing.

Mirrors cpp/ch25_international/international.h. Exchange rates are
domestic currency per unit of foreign currency.
"""

import math
from typing import Sequence


def currency_return(spot0: float, spot1: float) -> float:
    return spot1 / spot0 - 1


def domestic_return(foreign_return: float, currency_ret: float) -> float:
    return (1 + foreign_return) * (1 + currency_ret) - 1


def domestic_returns(foreign: Sequence[float],
                     currency: Sequence[float]) -> list[float]:
    if len(foreign) != len(currency):
        raise ValueError("size mismatch")
    return [domestic_return(r, e) for r, e in zip(foreign, currency)]


def domestic_return_variance(foreign_sd: float, currency_sd: float,
                             correlation: float) -> float:
    return (foreign_sd**2 + currency_sd**2 +
            2 * correlation * foreign_sd * currency_sd)


def parity_forward(spot: float, domestic_rate: float, foreign_rate: float,
                   years: float) -> float:
    return spot * ((1 + domestic_rate) / (1 + foreign_rate))**years


def forward_premium(spot: float, forward: float) -> float:
    return forward / spot - 1


def hedged_return(foreign_return: float, hedged_foreign_return: float,
                  spot0: float, forward: float, spot1: float) -> float:
    hedged = (1 + hedged_foreign_return) * forward
    unhedged = (foreign_return - hedged_foreign_return) * spot1
    return (hedged + unhedged) / spot0 - 1


def ppp_expected_change(domestic_inflation: float,
                        foreign_inflation: float) -> float:
    return (1 + domestic_inflation) / (1 + foreign_inflation) - 1


def real_exchange_rate(spot: float, foreign_price_level: float,
                       domestic_price_level: float) -> float:
    return spot * foreign_price_level / domestic_price_level


def two_market_std_dev(foreign_weight: float, domestic_sd: float,
                       foreign_sd: float, correlation: float) -> float:
    wd, wf = 1 - foreign_weight, foreign_weight
    return math.sqrt(wd * wd * domestic_sd**2 + wf * wf * foreign_sd**2 +
                     2 * wd * wf * correlation * domestic_sd * foreign_sd)


def minimum_risk_foreign_weight(domestic_sd: float, foreign_sd: float,
                                correlation: float) -> float:
    c = correlation * domestic_sd * foreign_sd
    denom = domestic_sd**2 + foreign_sd**2 - 2 * c
    if denom <= 0:
        raise ValueError("degenerate two-market problem")
    return (domestic_sd**2 - c) / denom
