// Chapter 2, Buying and Selling Securities.
// Section 2.4: margin accounts. Margin purchases (2.4.1), short sales (2.4.2)
// and aggregation of positions in one account (2.4.4).
//
// "Actual margin" is always the investor's equity divided by the market value
// of the position(s) the broker is exposed to. Initial margin applies when a
// position is opened; falling below maintenance margin triggers a margin call.

#ifndef CPP_CH02_BUYING_AND_SELLING_MARGIN_H_
#define CPP_CH02_BUYING_AND_SELLING_MARGIN_H_

#include <utility>
#include <vector>

namespace investm::ch02 {

// Buy `shares` at `price`, paying `initial_margin` of the cost and borrowing
// the rest from the broker.
struct MarginPurchase {
  double shares;
  double price;
  double initial_margin;  // For example 0.50.

  double Cost() const { return shares * price; }
  double EquityInvested() const { return Cost() * initial_margin; }
  double Loan() const { return Cost() - EquityInvested(); }

  // (n * P - loan) / (n * P) when the stock trades at P.
  double ActualMargin(double p) const;

  // Price at which actual margin hits `maintenance`:
  //   P = loan / (n * (1 - maintenance)).
  double MarginCallPrice(double maintenance) const;

  // Cash the investor must deposit at price `p` to restore `target` margin.
  double CashToRestore(double p, double target) const;

  // Return on the investor's own money: price change plus dividends, less
  // interest on the loan.
  double ReturnOnEquity(double end_price, double dividends_per_share = 0.0,
                        double loan_rate = 0.0) const;
};

// Borrow `shares`, sell them at `price`, leave the proceeds with the broker
// and deposit `initial_margin` of the proceeds as extra collateral.
struct ShortSale {
  double shares;
  double price;
  double initial_margin;

  double Proceeds() const { return shares * price; }
  double Deposit() const { return Proceeds() * initial_margin; }

  // (proceeds + deposit - n * P) / (n * P).
  double ActualMargin(double p) const;

  // Price at which actual margin hits `maintenance`:
  //   P = (proceeds + deposit) / (n * (1 + maintenance)).
  double MarginCallPrice(double maintenance) const;

  // Return on the deposit. The short seller pays any dividends on the
  // borrowed shares and may earn interest on the collateral.
  double ReturnOnEquity(double end_price, double dividends_per_share = 0.0,
                        double interest_on_collateral = 0.0) const;
};

struct Position {
  double shares;  // Positive for long positions, negative for short ones.
  double price;
};

// Several positions margined together (2.4.4).
class Account {
 public:
  Account(std::vector<Position> positions, double cash)
      : positions_(std::move(positions)), cash_(cash) {}

  void set_positions(std::vector<Position> positions) {
    positions_ = std::move(positions);
  }
  // Cash includes short-sale proceeds and deposits; negative means a loan.
  double cash() const { return cash_; }

  double LongValue() const;
  double ShortValue() const;
  double Equity() const { return cash_ + LongValue() - ShortValue(); }
  // Equity relative to the gross market value of all positions.
  double ActualMargin() const;

 private:
  std::vector<Position> positions_;
  double cash_;
};

}  // namespace investm::ch02

#endif  // CPP_CH02_BUYING_AND_SELLING_MARGIN_H_
