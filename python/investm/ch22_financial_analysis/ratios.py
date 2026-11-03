"""Chapter 22: financial ratios and the DuPont decomposition.

Mirrors cpp/ch22_financial_analysis/ratios.h.
"""

import dataclasses


@dataclasses.dataclass(frozen=True)
class Financials:
    """One year of statement items."""
    revenue: float
    ebit: float
    interest_expense: float
    income_tax: float
    total_assets: float
    equity: float
    current_assets: float
    inventory: float
    current_liabilities: float
    total_debt: float
    dividends: float
    shares: float
    price: float

    @property
    def pretax_income(self) -> float:
        return self.ebit - self.interest_expense

    @property
    def net_income(self) -> float:
        return self.pretax_income - self.income_tax


@dataclasses.dataclass(frozen=True)
class Ratios:
    """Profitability, liquidity, leverage and market ratios."""
    net_margin: float
    asset_turnover: float
    equity_multiplier: float
    roa: float
    roe: float
    current_ratio: float
    quick_ratio: float
    debt_to_equity: float
    interest_coverage: float
    payout_ratio: float
    eps: float
    price_earnings: float
    price_to_book: float
    dividend_yield: float


def compute_ratios(f: Financials) -> Ratios:
    ni = f.net_income
    eps = ni / f.shares
    return Ratios(
        net_margin=ni / f.revenue,
        asset_turnover=f.revenue / f.total_assets,
        equity_multiplier=f.total_assets / f.equity,
        roa=ni / f.total_assets,
        roe=ni / f.equity,
        current_ratio=f.current_assets / f.current_liabilities,
        quick_ratio=(f.current_assets - f.inventory) / f.current_liabilities,
        debt_to_equity=f.total_debt / f.equity,
        interest_coverage=(f.ebit / f.interest_expense
                           if f.interest_expense > 0 else 0.0),
        payout_ratio=f.dividends / ni if ni else 0.0,
        eps=eps,
        price_earnings=f.price / eps if eps else 0.0,
        price_to_book=f.price / (f.equity / f.shares),
        dividend_yield=f.dividends / f.shares / f.price if f.price else 0.0)


@dataclasses.dataclass(frozen=True)
class DuPont:
    tax_burden: float
    interest_burden: float
    operating_margin: float
    asset_turnover: float
    equity_multiplier: float

    @property
    def roe(self) -> float:
        return (self.tax_burden * self.interest_burden * self.operating_margin *
                self.asset_turnover * self.equity_multiplier)


def decompose_roe(f: Financials) -> DuPont:
    return DuPont(f.net_income / f.pretax_income, f.pretax_income / f.ebit,
                  f.ebit / f.revenue, f.revenue / f.total_assets,
                  f.total_assets / f.equity)
