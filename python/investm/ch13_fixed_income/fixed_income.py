"""Chapter 13: conventions for coupon bonds, callable bonds, preferreds.

Mirrors cpp/ch13_fixed_income/fixed_income.h.
"""

import calendar
import dataclasses
import datetime
import enum
from typing import Optional, Sequence

from investm.ch05_riskless_securities import cash_flows as cf
from investm.core import root_find

Date = datetime.date


class DayCount(enum.Enum):
    ACTUAL_ACTUAL = "act/act"
    THIRTY_360 = "30/360"
    ACTUAL_360 = "act/360"
    ACTUAL_365 = "act/365"


def _thirty_360(a: Date, b: Date) -> int:
    d1 = 30 if a.day == 31 else a.day
    d2 = 30 if (b.day == 31 and d1 == 30) else b.day
    return 360 * (b.year - a.year) + 30 * (b.month - a.month) + (d2 - d1)


def day_count_days(a: Date, b: Date, convention: DayCount) -> int:
    if a > b:
        raise ValueError("dates out of order")
    if convention == DayCount.THIRTY_360:
        return _thirty_360(a, b)
    return (b - a).days


def add_months(d: Date, months: int) -> Date:
    """Adds whole months, clamping to the end of the month."""
    total = d.year * 12 + (d.month - 1) + months
    year, month = divmod(total, 12)
    month += 1
    last = calendar.monthrange(year, month)[1]
    return Date(year, month, min(d.day, last))


@dataclasses.dataclass(frozen=True)
class Bond:
    face: float
    coupon_rate: float
    maturity: Date
    frequency: int = 2
    day_count: DayCount = DayCount.ACTUAL_ACTUAL


@dataclasses.dataclass(frozen=True)
class CouponPeriod:
    previous: Date
    next: Date
    remaining: int


def _step(frequency: int) -> int:
    if frequency <= 0 or 12 % frequency:
        raise ValueError("frequency must divide 12")
    return 12 // frequency


def surrounding_coupons(bond: Bond, settlement: Date) -> CouponPeriod:
    if settlement >= bond.maturity:
        raise ValueError("bond has matured")
    step = _step(bond.frequency)
    # k is the smallest count with add_months(maturity, -step (k + 1)) <=
    # settlement. Start one period short of the whole months to maturity and
    # correct by a step or two, instead of walking back from maturity.
    months_left = (12 * (bond.maturity.year - settlement.year) +
                   bond.maturity.month - settlement.month)
    k = max(0, months_left // step - 1)
    while k > 0 and add_months(bond.maturity, -step * k) <= settlement:
        k -= 1
    while add_months(bond.maturity, -step * (k + 1)) > settlement:
        k += 1
    return CouponPeriod(add_months(bond.maturity, -step * (k + 1)),
                        add_months(bond.maturity, -step * k), k + 1)


def _period_days(bond: Bond, p: CouponPeriod) -> float:
    if bond.day_count == DayCount.THIRTY_360:
        return 360.0 / bond.frequency
    return float(day_count_days(p.previous, p.next, bond.day_count))


def accrued_interest(bond: Bond, settlement: Date) -> float:
    p = surrounding_coupons(bond, settlement)
    coupon = bond.face * bond.coupon_rate / bond.frequency
    return coupon * day_count_days(p.previous, settlement,
                                   bond.day_count) / _period_days(bond, p)


def dirty_price(bond: Bond, settlement: Date, y: float) -> float:
    """Street convention: flow k discounted at (1 + y/f)^(k + w)."""
    p = surrounding_coupons(bond, settlement)
    coupon = bond.face * bond.coupon_rate / bond.frequency
    w = day_count_days(settlement, p.next, bond.day_count) / _period_days(
        bond, p)
    g = 1.0 + y / bond.frequency
    if g <= 0:
        raise ValueError("yield below -100% per period")
    pv = 0.0
    for k in range(p.remaining):
        flow = coupon + (bond.face if k + 1 == p.remaining else 0.0)
        pv += flow / g**(k + w)
    return pv


def clean_price(bond: Bond, settlement: Date, y: float) -> float:
    return dirty_price(bond, settlement, y) - accrued_interest(bond, settlement)


def yield_from_clean_price(bond: Bond, settlement: Date,
                           price: float) -> Optional[float]:
    if price <= 0:
        raise ValueError("price must be positive")
    target = price + accrued_interest(bond, settlement)
    return root_find.brent_expand_up(
        lambda y: dirty_price(bond, settlement, y) - target,
        -0.99 * bond.frequency, 1.0)


def price_from_32nds(handle: int, thirty_seconds: float) -> float:
    if not 0 <= thirty_seconds < 32:
        raise ValueError("32nds must be in [0, 32)")
    return handle + thirty_seconds / 32.0


@dataclasses.dataclass(frozen=True)
class CallDate:
    time: float
    price: float


def yield_to_call(face: float,
                  coupon_rate: float,
                  call: CallDate,
                  price: float,
                  frequency: int = 2) -> Optional[float]:
    flows = cf.coupon_bond(face, coupon_rate, call.time, frequency)
    last = flows[-1]
    flows[-1] = cf.CashFlow(last.time, last.amount + call.price - face)
    return cf.yield_to_maturity(flows, price, frequency)


@dataclasses.dataclass(frozen=True)
class YieldToWorstResult:
    yield_: float
    horizon: float
    is_maturity: bool


def yield_to_worst(face: float,
                   coupon_rate: float,
                   years_to_maturity: float,
                   calls: Sequence[CallDate],
                   price: float,
                   frequency: int = 2) -> Optional[YieldToWorstResult]:
    ytm = cf.yield_to_maturity(
        cf.coupon_bond(face, coupon_rate, years_to_maturity, frequency), price,
        frequency)
    if ytm is None:
        return None
    worst = YieldToWorstResult(ytm, years_to_maturity, True)
    for c in calls:
        if c.time >= years_to_maturity:
            continue
        y = yield_to_call(face, coupon_rate, c, price, frequency)
        if y is None:
            return None
        if y < worst.yield_ - 1e-12:
            worst = YieldToWorstResult(y, c.time, False)
    return worst


def perpetuity_value(dividend: float, required_return: float) -> float:
    if required_return <= 0:
        raise ValueError("required return must be positive")
    return dividend / required_return


def perpetuity_yield(dividend: float, price: float) -> float:
    if price <= 0:
        raise ValueError("price must be positive")
    return dividend / price
