// Chapter 19, Options.
// Sections 19.4-19.6: the binomial option pricing model (one-period
// replication and multi-period Cox-Ross-Rubinstein trees, European and
// American), the Black-Scholes model with Greeks, implied and historical
// volatility, and warrants.

#ifndef CPP_CH19_OPTIONS_PRICING_H_
#define CPP_CH19_OPTIONS_PRICING_H_

#include <optional>
#include <span>

#include "cpp/ch19_options/payoffs.h"

namespace investm::ch19 {

// 19.4.1 One-period binomial: the stock goes to spot*up or spot*down; the
// riskless return per period is 1 + rate. The option is replicated by
// `shares` of stock plus `bond` dollars of riskless lending (negative =
// borrowing).
struct Replication {
  double shares;  // Hedge ratio (delta).
  double bond;
  double value;
};
Replication OnePeriodBinomial(OptionType type, double spot, double strike,
                              double up, double down, double rate);

struct OptionSpec {
  OptionType type = OptionType::kCall;
  double spot = 0.0;
  double strike = 0.0;
  double rate = 0.0;            // Continuously compounded.
  double volatility = 0.0;      // Annualized.
  double years = 0.0;           // Time to expiry.
  double dividend_yield = 0.0;  // Continuous.
};

// 19.4.2 Cox-Ross-Rubinstein tree with `steps` periods.
double BinomialPrice(const OptionSpec& spec, int steps, bool american);

// 19.5 Black-Scholes(-Merton) price and sensitivities.
struct Greeks {
  double price;
  double delta;
  double gamma;
  double vega;   // Per 1.00 change in volatility.
  double theta;  // Per year.
  double rho;    // Per 1.00 change in the rate.
};
double BlackScholesPrice(const OptionSpec& spec);
Greeks BlackScholesGreeks(const OptionSpec& spec);

// 19.5.4 Volatility that reproduces `price`; nullopt if outside the
// no-arbitrage range.
std::optional<double> ImpliedVolatility(OptionSpec spec, double price);
// Annualized standard deviation of log price relatives.
double HistoricalVolatility(std::span<const double> prices,
                            double periods_per_year);

// 19.7 Warrants: with N shares and M warrants, each convertible into one
// share at `strike`, a warrant is worth N / (N + M) calls on the post-
// exercise share value. `spec.spot` is the current share price (equity
// value / N, excluding warrant proceeds).
double WarrantValue(const OptionSpec& spec, double shares, double warrants);

}  // namespace investm::ch19

#endif  // CPP_CH19_OPTIONS_PRICING_H_
