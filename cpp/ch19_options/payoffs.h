// Chapter 19, Options.
// Sections 19.1-19.3: option payoffs and profits at expiration, option
// strategies built from legs, put-call parity and no-arbitrage bounds.

#ifndef CPP_CH19_OPTIONS_PAYOFFS_H_
#define CPP_CH19_OPTIONS_PAYOFFS_H_

#include <span>

namespace investm::ch19 {

enum class OptionType { kCall, kPut };

double Payoff(OptionType type, double strike, double spot_at_expiry);

// One position in a strategy. A negative quantity is a short (written)
// position. For kStock the strike is ignored and `premium` is the purchase
// price; for kBond the position pays `strike` at expiry and costs `premium`.
enum class Instrument { kCall, kPut, kStock, kBond };
struct Leg {
  Instrument instrument;
  double strike;
  double quantity;
  double premium;  // Price paid per unit today.
};

// Value of the legs at expiry, and value net of the initial cost (ignoring
// interest on the premiums).
double StrategyPayoff(std::span<const Leg> legs, double spot_at_expiry);
double StrategyProfit(std::span<const Leg> legs, double spot_at_expiry);

// 19.3 Put-call parity for European options on a non-dividend stock with
// continuously compounded rate r: C - P = S - K e^{-rT}.
double ParityPutPrice(double call, double spot, double strike, double rate,
                      double years);
double ParityCallPrice(double put, double spot, double strike, double rate,
                       double years);
// C - P - (S - K e^{-rT}); nonzero values signal an arbitrage opportunity.
double ParityDeviation(double call, double put, double spot, double strike,
                       double rate, double years);

// No-arbitrage bounds for European options.
double CallLowerBound(double spot, double strike, double rate, double years);
double PutLowerBound(double spot, double strike, double rate, double years);

}  // namespace investm::ch19

#endif  // CPP_CH19_OPTIONS_PAYOFFS_H_
