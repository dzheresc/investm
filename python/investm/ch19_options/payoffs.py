"""Chapter 19, sections 19.1-19.3: payoffs, strategies and parity.

Mirrors cpp/ch19_options/payoffs.h.
"""

import dataclasses
import enum
import math
from typing import Sequence


class OptionType(enum.Enum):
    CALL = "call"
    PUT = "put"


class Instrument(enum.Enum):
    CALL = "call"
    PUT = "put"
    STOCK = "stock"
    BOND = "bond"


def payoff(option_type: OptionType, strike: float, spot: float) -> float:
    if option_type == OptionType.CALL:
        return max(spot - strike, 0.0)
    return max(strike - spot, 0.0)


@dataclasses.dataclass(frozen=True)
class Leg:
    """Negative quantity = short. A bond pays `strike` at expiry."""
    instrument: Instrument
    strike: float
    quantity: float
    premium: float


def strategy_payoff(legs: Sequence[Leg], spot: float) -> float:
    total = 0.0
    for leg in legs:
        if leg.instrument == Instrument.CALL:
            unit = payoff(OptionType.CALL, leg.strike, spot)
        elif leg.instrument == Instrument.PUT:
            unit = payoff(OptionType.PUT, leg.strike, spot)
        elif leg.instrument == Instrument.STOCK:
            unit = spot
        else:
            unit = leg.strike
        total += leg.quantity * unit
    return total


def strategy_profit(legs: Sequence[Leg], spot: float) -> float:
    return strategy_payoff(legs, spot) - sum(
        l.quantity * l.premium for l in legs)


def parity_put_price(call: float, spot: float, strike: float, rate: float,
                     years: float) -> float:
    return call - spot + strike * math.exp(-rate * years)


def parity_call_price(put: float, spot: float, strike: float, rate: float,
                      years: float) -> float:
    return put + spot - strike * math.exp(-rate * years)


def parity_deviation(call: float, put: float, spot: float, strike: float,
                     rate: float, years: float) -> float:
    return call - put - (spot - strike * math.exp(-rate * years))


def call_lower_bound(spot: float, strike: float, rate: float,
                     years: float) -> float:
    return max(spot - strike * math.exp(-rate * years), 0.0)


def put_lower_bound(spot: float, strike: float, rate: float,
                    years: float) -> float:
    return max(strike * math.exp(-rate * years) - spot, 0.0)
