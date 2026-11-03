"""Chapter 2, section 2.4: margin purchases, short sales, aggregation.

"Actual margin" is the investor's equity divided by the market value of the
position(s) the broker is exposed to. Mirrors
cpp/ch02_buying_and_selling/margin.h.
"""

import dataclasses
from typing import Sequence


@dataclasses.dataclass(frozen=True)
class MarginPurchase:
    """Buy shares paying `initial_margin` of the cost, borrowing the rest."""
    shares: float
    price: float
    initial_margin: float

    @property
    def cost(self) -> float:
        return self.shares * self.price

    @property
    def equity_invested(self) -> float:
        return self.cost * self.initial_margin

    @property
    def loan(self) -> float:
        return self.cost - self.equity_invested

    def actual_margin(self, p: float) -> float:
        value = self.shares * p
        return (value - self.loan) / value

    def margin_call_price(self, maintenance: float) -> float:
        """P = loan / (n * (1 - maintenance))."""
        if maintenance >= 1.0:
            raise ValueError("maintenance margin must be below 1")
        return self.loan / (self.shares * (1.0 - maintenance))

    def cash_to_restore(self, p: float, target: float) -> float:
        value = self.shares * p
        return max(0.0, target * value - (value - self.loan))

    def return_on_equity(self,
                         end_price: float,
                         dividends_per_share: float = 0.0,
                         loan_rate: float = 0.0) -> float:
        gain = self.shares * (end_price - self.price + dividends_per_share)
        return (gain - self.loan * loan_rate) / self.equity_invested


@dataclasses.dataclass(frozen=True)
class ShortSale:
    """Sell borrowed shares; proceeds plus a deposit stay with the broker."""
    shares: float
    price: float
    initial_margin: float

    @property
    def proceeds(self) -> float:
        return self.shares * self.price

    @property
    def deposit(self) -> float:
        return self.proceeds * self.initial_margin

    def actual_margin(self, p: float) -> float:
        owed = self.shares * p
        return (self.proceeds + self.deposit - owed) / owed

    def margin_call_price(self, maintenance: float) -> float:
        """P = (proceeds + deposit) / (n * (1 + maintenance))."""
        return (self.proceeds + self.deposit) / (self.shares *
                                                 (1.0 + maintenance))

    def return_on_equity(self,
                         end_price: float,
                         dividends_per_share: float = 0.0,
                         interest_on_collateral: float = 0.0) -> float:
        gain = self.shares * (self.price - end_price - dividends_per_share)
        return (gain + interest_on_collateral) / self.deposit


@dataclasses.dataclass(frozen=True)
class Position:
    shares: float  # Negative for a short position.
    price: float


@dataclasses.dataclass
class Account:
    """Several positions margined together (2.4.4)."""
    positions: Sequence[Position]
    cash: float  # Includes short proceeds and deposits; negative is a loan.

    @property
    def long_value(self) -> float:
        return sum(p.shares * p.price for p in self.positions if p.shares > 0)

    @property
    def short_value(self) -> float:
        return sum(-p.shares * p.price for p in self.positions if p.shares < 0)

    @property
    def equity(self) -> float:
        return self.cash + self.long_value - self.short_value

    def actual_margin(self) -> float:
        gross = self.long_value + self.short_value
        if gross <= 0.0:
            raise ValueError("account has no positions")
        return self.equity / gross
