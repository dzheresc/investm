"""Chapter 12, section 12.1: taxes.

Mirrors cpp/ch12_taxes_inflation/taxes.h.
"""

import dataclasses
from typing import Sequence


@dataclasses.dataclass(frozen=True)
class TaxBracket:
    threshold: float
    rate: float


class TaxSchedule:
    """A progressive schedule; brackets start at 0 and increase."""

    def __init__(self, brackets: Sequence[TaxBracket]):
        if not brackets or brackets[0].threshold != 0.0:
            raise ValueError("first bracket must start at zero")
        for a, b in zip(brackets, brackets[1:]):
            if b.threshold <= a.threshold:
                raise ValueError("thresholds must increase")
        self._brackets = list(brackets)

    def tax(self, income: float) -> float:
        if income < 0:
            raise ValueError("income must be non-negative")
        total = 0.0
        for i, b in enumerate(self._brackets):
            if income <= b.threshold:
                break
            hi = (self._brackets[i + 1].threshold if i +
                  1 < len(self._brackets) else income)
            total += (min(income, hi) - b.threshold) * b.rate
        return total

    def marginal_rate(self, income: float) -> float:
        rate = self._brackets[0].rate
        for b in self._brackets:
            if income >= b.threshold:
                rate = b.rate
        return rate

    def average_rate(self, income: float) -> float:
        return self.tax(income) / income if income > 0 else 0.0


def after_tax_return(pretax: float, tax_rate: float) -> float:
    return pretax * (1.0 - tax_rate)


def combined_tax_rate(federal: float, state: float,
                      state_deductible: bool) -> float:
    if not (0 <= federal < 1 and 0 <= state < 1):
        raise ValueError("rates must be in [0, 1)")
    if state_deductible:
        return federal + state * (1.0 - federal)
    return federal + state


def taxable_equivalent_yield(tax_exempt_yield: float, tax_rate: float) -> float:
    if not 0 <= tax_rate < 1:
        raise ValueError("tax rate must be in [0, 1)")
    return tax_exempt_yield / (1.0 - tax_rate)


def breakeven_tax_rate(taxable_yield: float, tax_exempt_yield: float) -> float:
    return 1.0 - tax_exempt_yield / taxable_yield


def wealth_taxed_annually(amount: float, r: float, tax_rate: float,
                          years: float) -> float:
    return amount * (1.0 + r * (1.0 - tax_rate))**years


def wealth_taxed_at_end(amount: float, r: float, tax_rate: float,
                        years: float) -> float:
    gross = amount * (1.0 + r)**years
    return gross - tax_rate * (gross - amount)


def wealth_untaxed(amount: float, r: float, years: float) -> float:
    return amount * (1.0 + r)**years


def effective_deferred_rate(r: float, tax_rate: float, years: float) -> float:
    if years <= 0:
        raise ValueError("horizon must be positive")
    return wealth_taxed_at_end(1.0, r, tax_rate, years)**(1.0 / years) - 1.0
