"""Chapter 2, section 2.3: market, limit, stop and stop-limit orders.

A toy execution simulator mirroring cpp/ch02_buying_and_selling/orders.h.
"""

import dataclasses
import enum
from typing import Optional, Sequence


class Side(enum.Enum):
    BUY = "buy"
    SELL = "sell"


class OrderType(enum.Enum):
    MARKET = "market"
    LIMIT = "limit"
    STOP = "stop"
    STOP_LIMIT = "stop_limit"


@dataclasses.dataclass(frozen=True)
class Order:
    side: Side = Side.BUY
    type: OrderType = OrderType.MARKET
    limit: float = 0.0  # Used by LIMIT and STOP_LIMIT.
    stop: float = 0.0  # Used by STOP and STOP_LIMIT.


@dataclasses.dataclass(frozen=True)
class Fill:
    index: int
    price: float


def _limit_ok(side: Side, price: float, limit: float) -> bool:
    return price <= limit if side is Side.BUY else price >= limit


def _stop_hit(side: Side, price: float, stop: float) -> bool:
    return price >= stop if side is Side.BUY else price <= stop


def simulate(order: Order, path: Sequence[float]) -> Optional[Fill]:
    """Walks `path` with a good-till-cancelled order.

    Buy stops trigger on a rise, sell stops (stop-loss) on a fall. A triggered
    stop becomes a market order; a triggered stop limit becomes a limit order.

    Returns:
        The fill, or None if the order never executes.
    """
    if not path:
        raise ValueError("empty price path")
    active = order.type in (OrderType.MARKET, OrderType.LIMIT)
    for i, price in enumerate(path):
        if not active:
            if not _stop_hit(order.side, price, order.stop):
                continue
            active = True
        if order.type in (OrderType.MARKET, OrderType.STOP):
            return Fill(i, price)
        if _limit_ok(order.side, price, order.limit):
            return Fill(i, price)
    return None
