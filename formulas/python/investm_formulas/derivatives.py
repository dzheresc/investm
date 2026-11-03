"""Options and futures (equations 55 to 60)."""

import math

__all__ = [
    "call_intrinsic_value",
    "put_intrinsic_value",
    "put_from_parity",
    "call_from_parity",
    "normal_cdf",
    "black_scholes_d1",
    "black_scholes_d2",
    "black_scholes_call",
    "black_scholes_put",
    "futures_fair_value",
    "index_futures_fair_value",
]


def call_intrinsic_value(stock_price: float, strike: float) -> float:
    """Eq. 55: IV_C = max(0, P_S - E)."""
    return max(0.0, stock_price - strike)


def put_intrinsic_value(stock_price: float, strike: float) -> float:
    """Eq. 55: IV_P = max(0, E - P_S)."""
    return max(0.0, strike - stock_price)


def put_from_parity(call_price: float, stock_price: float, strike: float,
                    rate: float, years: float) -> float:
    """Eq. 56 solved for the put: P_P = P_C + E / e^(RT) - P_S."""
    return call_price + strike * math.exp(-rate * years) - stock_price


def call_from_parity(put_price: float, stock_price: float, strike: float,
                     rate: float, years: float) -> float:
    """Eq. 56 solved for the call: P_C = P_P + P_S - E / e^(RT)."""
    return put_price + stock_price - strike * math.exp(-rate * years)


def normal_cdf(x: float) -> float:
    """Standard normal cumulative distribution N(x)."""
    return 0.5 * math.erfc(-x / math.sqrt(2.0))


def black_scholes_d1(stock_price: float, strike: float, rate: float,
                     years: float, volatility: float) -> float:
    """Eq. 57: d1 = [ln(P_S/E) + (R + 0.5 sigma^2) T] / (sigma sqrt T)."""
    return (math.log(stock_price / strike) +
            (rate + 0.5 * volatility**2) * years) / (volatility *
                                                     math.sqrt(years))


def black_scholes_d2(stock_price: float, strike: float, rate: float,
                     years: float, volatility: float) -> float:
    """Eq. 57: d2 = d1 - sigma sqrt T."""
    return black_scholes_d1(stock_price, strike, rate, years,
                            volatility) - volatility * math.sqrt(years)


def black_scholes_call(stock_price: float, strike: float, rate: float,
                       years: float, volatility: float) -> float:
    """Eq. 57: V_C = N(d1) P_S - E / e^(RT) N(d2)."""
    d1 = black_scholes_d1(stock_price, strike, rate, years, volatility)
    d2 = d1 - volatility * math.sqrt(years)
    return normal_cdf(d1) * stock_price - \
        strike * math.exp(-rate * years) * normal_cdf(d2)


def black_scholes_put(stock_price: float, strike: float, rate: float,
                      years: float, volatility: float) -> float:
    """Eq. 58: P_P = E / e^(RT) N(-d2) - P_S N(-d1)."""
    d1 = black_scholes_d1(stock_price, strike, rate, years, volatility)
    d2 = d1 - volatility * math.sqrt(years)
    return strike * math.exp(-rate * years) * normal_cdf(-d2) - \
        stock_price * normal_cdf(-d1)


def futures_fair_value(spot_price: float, interest: float, benefits: float,
                       costs: float) -> float:
    """Eq. 59: P_f = P_s + I - B + C."""
    return spot_price + interest - benefits + costs


def index_futures_fair_value(spot_price: float, rate: float,
                             dividend_yield: float) -> float:
    """Eq. 60: P_f = P_s + R P_s - y P_s."""
    return spot_price + rate * spot_price - dividend_yield * spot_price
