// Chapter 3, Security Markets.
// Sections 3.7 (commissions) and 3.8 (transactions costs: bid-ask spread and
// price impact).

#ifndef CPP_CH03_SECURITY_MARKETS_COSTS_H_
#define CPP_CH03_SECURITY_MARKETS_COSTS_H_

namespace investm::ch03 {

// A commission schedule: fixed ticket charge, per-share charge, percentage of
// principal and a minimum. Any field may be zero.
struct CommissionSchedule {
  double per_ticket = 0.0;
  double per_share = 0.0;
  double fraction_of_principal = 0.0;
  double minimum = 0.0;

  double Commission(double shares, double price) const;
};

struct Quote {
  double bid;
  double ask;

  double Mid() const { return 0.5 * (bid + ask); }
  double Spread() const { return ask - bid; }
  // Cost of an immediate round trip, as a fraction of the midpoint.
  double RelativeSpread() const { return Spread() / Mid(); }
};

// |trade price - mid| / mid. Below the quoted half-spread when a trade prints
// inside the quote.
double EffectiveHalfSpread(double trade_price, const Quote& quote);

// Stylized square-root price-impact model widely used in practice (not from
// the book): impact = k * daily_vol * sqrt(shares / average_daily_volume).
double SqrtPriceImpact(double shares, double average_daily_volume,
                       double daily_vol, double k = 1.0);

// One-way cost as a fraction of principal: commission, half the spread and
// price impact.
double OneWayCost(double shares, double price, const Quote& quote,
                  const CommissionSchedule& schedule, double impact_fraction);

}  // namespace investm::ch03

#endif  // CPP_CH03_SECURITY_MARKETS_COSTS_H_
