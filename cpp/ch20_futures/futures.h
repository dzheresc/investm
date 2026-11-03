// Chapter 20, Futures.
// Marking to market and margin calls, the cost-of-carry relation between
// spot and futures prices with its arbitrage bounds, basis and convergence,
// hedging with futures (minimum-variance and beta hedges), and options on
// futures (Black's model).

#ifndef CPP_CH20_FUTURES_FUTURES_H_
#define CPP_CH20_FUTURES_FUTURES_H_

#include <span>
#include <vector>

#include "cpp/ch19_options/payoffs.h"
#include "cpp/core/stats.h"

namespace investm::ch20 {

// 20.2 Daily settlement of a futures position. `position` is the number of
// contracts (negative = short); each settlement price change moves equity
// by position * multiplier * dF. Whenever equity falls below the
// maintenance margin, the holder deposits enough to restore the initial
// margin.
struct MarginDay {
  double settlement_price;
  double gain;         // Mark-to-market gain for the day.
  double margin_call;  // Deposit required at the close (0 if none).
  double equity;       // Account equity after any deposit.
};
std::vector<MarginDay> MarkToMarket(double entry_price,
                                    std::span<const double> settlements,
                                    double position, double multiplier,
                                    double initial_margin,
                                    double maintenance_margin);

// 20.5 Cost of carry with continuous compounding:
//   F = S exp((r + storage - yield) T).
double CostOfCarryPrice(double spot, double rate, double income_yield,
                        double storage_cost, double years);
// Discrete-income version for a stock index: F = (S - PV(dividends)) e^{rT},
// with each dividend (time, amount) discounted at r.
struct Dividend {
  double time;
  double amount;
};
double IndexFuturesPrice(double spot, double rate,
                         std::span<const Dividend> dividends, double years);

// No-arbitrage band when borrowing and lending rates differ and a round
// trip costs `cost` (as a fraction of spot): futures outside the band can
// be arbitraged by cash-and-carry or reverse cash-and-carry.
struct ArbitrageBand {
  double lower;
  double upper;
};
ArbitrageBand NoArbitrageBand(double spot, double lending_rate,
                              double borrowing_rate, double income_yield,
                              double cost, double years);

// Basis = spot - futures.
inline double Basis(double spot, double futures) { return spot - futures; }

// 20.4 Hedging. Minimum-variance hedge ratio: OLS slope of spot changes on
// futures changes; the hedge sells h units of futures per unit of spot.
stats::SimpleRegression MinimumVarianceHedge(
    std::span<const double> spot_changes,
    std::span<const double> futures_changes);
// Variance of the hedged position (dS - h dF) relative to unhedged.
double HedgeEffectiveness(std::span<const double> spot_changes,
                          std::span<const double> futures_changes, double h);
// Contracts needed to hedge `exposure` units of spot with futures of size
// `contract_size`: h * exposure / contract_size.
double ContractsToHedge(double hedge_ratio, double exposure,
                        double contract_size);
// Stock-index futures contracts to move a portfolio from beta `current` to
// beta `target`: (target - current) * value / (futures * multiplier).
// Negative = sell.
double IndexFuturesContracts(double portfolio_value, double current_beta,
                             double target_beta, double futures_price,
                             double multiplier);

// 20.6 Black's (1976) model for European options on futures.
double BlackFuturesOptionPrice(ch19::OptionType type, double futures,
                               double strike, double rate, double volatility,
                               double years);

}  // namespace investm::ch20

#endif  // CPP_CH20_FUTURES_FUTURES_H_
