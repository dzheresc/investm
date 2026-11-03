#include "cpp/ch02_buying_and_selling/margin.h"

#include "cpp/core/check.h"

namespace investm::ch02 {

double MarginPurchase::ActualMargin(double p) const {
  const double value = shares * p;
  INVESTM_CHECK(value > 0.0, "position value must be positive");
  return (value - Loan()) / value;
}

double MarginPurchase::MarginCallPrice(double maintenance) const {
  INVESTM_CHECK(maintenance < 1.0, "maintenance margin must be below 1");
  return Loan() / (shares * (1.0 - maintenance));
}

double MarginPurchase::CashToRestore(double p, double target) const {
  const double value = shares * p;
  const double needed = target * value;
  const double equity = value - Loan();
  return needed > equity ? needed - equity : 0.0;
}

double MarginPurchase::ReturnOnEquity(double end_price,
                                      double dividends_per_share,
                                      double loan_rate) const {
  const double gain = shares * (end_price - price + dividends_per_share);
  return (gain - Loan() * loan_rate) / EquityInvested();
}

double ShortSale::ActualMargin(double p) const {
  const double owed = shares * p;
  INVESTM_CHECK(owed > 0.0, "position value must be positive");
  return (Proceeds() + Deposit() - owed) / owed;
}

double ShortSale::MarginCallPrice(double maintenance) const {
  return (Proceeds() + Deposit()) / (shares * (1.0 + maintenance));
}

double ShortSale::ReturnOnEquity(double end_price, double dividends_per_share,
                                 double interest_on_collateral) const {
  const double gain = shares * (price - end_price - dividends_per_share);
  return (gain + interest_on_collateral) / Deposit();
}

double Account::LongValue() const {
  double v = 0.0;
  for (const Position& p : positions_) {
    if (p.shares > 0.0) v += p.shares * p.price;
  }
  return v;
}

double Account::ShortValue() const {
  double v = 0.0;
  for (const Position& p : positions_) {
    if (p.shares < 0.0) v -= p.shares * p.price;
  }
  return v;
}

double Account::ActualMargin() const {
  const double gross = LongValue() + ShortValue();
  INVESTM_CHECK(gross > 0.0, "account has no positions");
  return Equity() / gross;
}

}  // namespace investm::ch02
