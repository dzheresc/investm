"""Chapter 5, section 5.2: present value and yield-to-maturity.

Mirrors cpp/ch05_riskless_securities/cash_flows.h.
"""

import dataclasses
import math
from typing import Optional, Sequence

from investm.core import root_find


@dataclasses.dataclass(frozen=True)
class CashFlow:
    time: float  # Years from today.
    amount: float


def coupon_bond(face: float,
                coupon_rate: float,
                years: float,
                frequency: int = 1) -> list[CashFlow]:
    """Promised cash flows of a level-coupon bullet bond."""
    if frequency <= 0 or years <= 0:
        raise ValueError("invalid bond terms")
    n = round(years * frequency)
    coupon = face * coupon_rate / frequency
    return [
        CashFlow(k / frequency, coupon + (face if k == n else 0.0))
        for k in range(1, n + 1)
    ]


def zero_coupon(face: float, years: float) -> list[CashFlow]:
    return [CashFlow(years, face)]


def present_value_at_yield(flows: Sequence[CashFlow],
                           y: float,
                           frequency: int = 1) -> float:
    """PV at a flat yield quoted with `frequency` compounding per year."""
    log_growth = math.log1p(y / frequency) * frequency
    return math.fsum(
        cf.amount * math.exp(-log_growth * cf.time) for cf in flows)


def present_value_continuous(flows: Sequence[CashFlow], y: float) -> float:
    return math.fsum(cf.amount * math.exp(-y * cf.time) for cf in flows)


def yield_to_maturity(flows: Sequence[CashFlow],
                      price: float,
                      frequency: int = 1) -> Optional[float]:
    """Solves PV(y) = price; None if no yield above -100% works."""
    if price <= 0.0:
        raise ValueError("price must be positive")
    return root_find.brent_expand_up(
        lambda y: present_value_at_yield(flows, y, frequency) - price,
        -0.99 * frequency, 0.20)


def internal_rate_of_return(flows: Sequence[CashFlow],
                            lo: float = -0.99,
                            hi: float = 1.0) -> Optional[float]:
    """IRR (annual compounding) of a stream including the time-0 outlay."""
    return root_find.brent(lambda r: present_value_at_yield(flows, r), lo, hi)
