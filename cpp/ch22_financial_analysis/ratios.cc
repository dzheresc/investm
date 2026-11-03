#include "cpp/ch22_financial_analysis/ratios.h"

#include "cpp/core/check.h"

namespace investm::ch22 {

Ratios ComputeRatios(const Financials& f) {
  INVESTM_CHECK(f.revenue > 0.0 && f.total_assets > 0.0 && f.equity > 0.0,
                "revenue, assets and equity must be positive");
  INVESTM_CHECK(f.current_liabilities > 0.0 && f.shares > 0.0,
                "current liabilities and shares must be positive");
  const double ni = f.NetIncome();
  Ratios r{};
  r.net_margin = ni / f.revenue;
  r.asset_turnover = f.revenue / f.total_assets;
  r.equity_multiplier = f.total_assets / f.equity;
  r.roa = ni / f.total_assets;
  r.roe = ni / f.equity;
  r.current_ratio = f.current_assets / f.current_liabilities;
  r.quick_ratio = (f.current_assets - f.inventory) / f.current_liabilities;
  r.debt_to_equity = f.total_debt / f.equity;
  r.interest_coverage =
      f.interest_expense > 0.0 ? f.ebit / f.interest_expense : 0.0;
  r.payout_ratio = ni != 0.0 ? f.dividends / ni : 0.0;
  r.eps = ni / f.shares;
  r.price_earnings = r.eps != 0.0 ? f.price / r.eps : 0.0;
  r.price_to_book = f.price / (f.equity / f.shares);
  r.dividend_yield = f.price > 0.0 ? f.dividends / f.shares / f.price : 0.0;
  return r;
}

DuPont DecomposeRoe(const Financials& f) {
  INVESTM_CHECK(f.ebit != 0.0 && f.PretaxIncome() != 0.0,
                "EBIT and pretax income must be nonzero");
  INVESTM_CHECK(f.revenue > 0.0 && f.total_assets > 0.0 && f.equity > 0.0,
                "revenue, assets and equity must be positive");
  return {f.NetIncome() / f.PretaxIncome(), f.PretaxIncome() / f.ebit,
          f.ebit / f.revenue, f.revenue / f.total_assets,
          f.total_assets / f.equity};
}

}  // namespace investm::ch22
