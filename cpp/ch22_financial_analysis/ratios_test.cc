#include "cpp/ch22_financial_analysis/ratios.h"

#include "gtest/gtest.h"

namespace investm::ch22 {
namespace {

Financials Sample() {
  Financials f;
  f.revenue = 1000.0;
  f.ebit = 150.0;
  f.interest_expense = 30.0;
  f.income_tax = 30.0;
  f.total_assets = 800.0;
  f.equity = 400.0;
  f.current_assets = 300.0;
  f.inventory = 100.0;
  f.current_liabilities = 150.0;
  f.total_debt = 250.0;
  f.dividends = 36.0;
  f.shares = 50.0;
  f.price = 27.0;
  return f;
}

TEST(RatiosTest, Basic) {
  const Ratios r = ComputeRatios(Sample());
  EXPECT_NEAR(r.net_margin, 0.09, 1e-15);
  EXPECT_NEAR(r.asset_turnover, 1.25, 1e-15);
  EXPECT_NEAR(r.equity_multiplier, 2.0, 1e-15);
  EXPECT_NEAR(r.roe, 0.225, 1e-15);
  EXPECT_NEAR(r.roe, r.net_margin * r.asset_turnover * r.equity_multiplier,
              1e-15);
  EXPECT_NEAR(r.current_ratio, 2.0, 1e-15);
  EXPECT_NEAR(r.quick_ratio, 200.0 / 150.0, 1e-15);
  EXPECT_NEAR(r.interest_coverage, 5.0, 1e-15);
  EXPECT_NEAR(r.payout_ratio, 0.4, 1e-15);
  EXPECT_NEAR(r.eps, 1.8, 1e-15);
  EXPECT_NEAR(r.price_earnings, 15.0, 1e-12);
  EXPECT_NEAR(r.price_to_book, 27.0 / 8.0, 1e-15);
  EXPECT_NEAR(r.dividend_yield, 0.72 / 27.0, 1e-15);
}

TEST(RatiosTest, FiveFactorDuPont) {
  const Financials f = Sample();
  const DuPont d = DecomposeRoe(f);
  EXPECT_NEAR(d.tax_burden, 0.75, 1e-15);
  EXPECT_NEAR(d.interest_burden, 0.8, 1e-15);
  EXPECT_NEAR(d.operating_margin, 0.15, 1e-15);
  EXPECT_NEAR(d.Roe(), ComputeRatios(f).roe, 1e-15);
}

}  // namespace
}  // namespace investm::ch22
