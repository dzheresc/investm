// Chapter 22, Financial Analysis.
// Fundamental analysis with financial ratios: profitability, the DuPont
// decomposition of return on equity, liquidity, leverage and market ratios.

#ifndef CPP_CH22_FINANCIAL_ANALYSIS_RATIOS_H_
#define CPP_CH22_FINANCIAL_ANALYSIS_RATIOS_H_

namespace investm::ch22 {

// A minimal set of statement items (one year; balance sheet items are the
// values used as denominators, e.g. averages or year-end).
struct Financials {
  double revenue = 0.0;
  double ebit = 0.0;  // Operating income.
  double interest_expense = 0.0;
  double income_tax = 0.0;
  double total_assets = 0.0;
  double equity = 0.0;
  double current_assets = 0.0;
  double inventory = 0.0;
  double current_liabilities = 0.0;
  double total_debt = 0.0;
  double dividends = 0.0;
  double shares = 0.0;
  double price = 0.0;

  double PretaxIncome() const { return ebit - interest_expense; }
  double NetIncome() const { return PretaxIncome() - income_tax; }
};

struct Ratios {
  double net_margin;
  double asset_turnover;
  double equity_multiplier;  // Assets / equity.
  double roa;                // Net income / assets.
  double roe;
  double current_ratio;
  double quick_ratio;
  double debt_to_equity;
  double interest_coverage;  // EBIT / interest.
  double payout_ratio;
  double eps;
  double price_earnings;
  double price_to_book;
  double dividend_yield;
};
Ratios ComputeRatios(const Financials& f);

// Five-factor DuPont: ROE = tax burden x interest burden x operating margin
// x asset turnover x equity multiplier, where tax burden = NI / pretax,
// interest burden = pretax / EBIT and operating margin = EBIT / sales.
struct DuPont {
  double tax_burden;
  double interest_burden;
  double operating_margin;
  double asset_turnover;
  double equity_multiplier;
  double Roe() const {
    return tax_burden * interest_burden * operating_margin * asset_turnover *
           equity_multiplier;
  }
};
DuPont DecomposeRoe(const Financials& f);

}  // namespace investm::ch22

#endif  // CPP_CH22_FINANCIAL_ANALYSIS_RATIOS_H_
