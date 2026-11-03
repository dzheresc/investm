// Worked examples for Chapter 19. All numbers are illustrative.

#include <cstdio>
#include <vector>

#include "cpp/ch19_options/payoffs.h"
#include "cpp/ch19_options/pricing.h"
#include "cpp/core/table.h"

namespace {

using investm::PrintHeading;
using investm::PrintPercent;
using investm::PrintValue;
namespace ch19 = investm::ch19;
using ch19::Instrument;
using ch19::OptionType;

void Strategies() {
  PrintHeading("19.2 Profit at expiry of some option strategies");
  const std::vector<ch19::Leg> bull{{Instrument::kCall, 45.0, 1.0, 4.10},
                                    {Instrument::kCall, 55.0, -1.0, 1.20}};
  const std::vector<ch19::Leg> straddle{{Instrument::kCall, 50.0, 1.0, 2.60},
                                        {Instrument::kPut, 50.0, 1.0, 2.25}};
  const std::vector<ch19::Leg> protective{{Instrument::kStock, 0.0, 1.0, 50.0},
                                          {Instrument::kPut, 45.0, 1.0, 0.90}};
  const std::vector<ch19::Leg> covered{{Instrument::kStock, 0.0, 1.0, 50.0},
                                       {Instrument::kCall, 55.0, -1.0, 1.20}};
  std::printf("  %8s %10s %10s %12s %12s\n", "S_T", "bull call", "straddle",
              "protective", "covered");
  for (double s = 35.0; s <= 65.0; s += 5.0) {
    std::printf(
        "  %8.1f %10.2f %10.2f %12.2f %12.2f\n", s,
        ch19::StrategyProfit(bull, s), ch19::StrategyProfit(straddle, s),
        ch19::StrategyProfit(protective, s), ch19::StrategyProfit(covered, s));
  }
}

void Parity() {
  PrintHeading("19.3 Put-call parity (S 62, K 60, r 4.5%, 4 months)");
  const double t = 4.0 / 12.0;
  const double put = ch19::ParityPutPrice(5.10, 62.0, 60.0, 0.045, t);
  PrintValue("put implied by a $5.10 call", put);
  PrintValue("parity deviation if the put trades at $2.50",
             ch19::ParityDeviation(5.10, 2.50, 62.0, 60.0, 0.045, t));
  PrintValue("call lower bound", ch19::CallLowerBound(62.0, 60.0, 0.045, t));
}

void Binomial() {
  PrintHeading("19.4 Binomial model");
  const auto r =
      ch19::OnePeriodBinomial(OptionType::kCall, 80.0, 80.0, 1.25, 0.85, 0.03);
  std::printf("  one period, S 80 -> 100 or 68, K 80, r 3%%:\n");
  std::printf("  buy %.4f shares, borrow %.4f -> call worth %.4f\n", r.shares,
              -r.bond, r.value);
  const ch19::OptionSpec put{
      OptionType::kPut, 48.0, 50.0, 0.05, 0.32, 0.5, 0.0};
  std::printf("  %8s %12s %12s\n", "steps", "European", "American");
  for (int n : {1, 2, 5, 20, 100, 500}) {
    std::printf("  %8d %12.5f %12.5f\n", n, ch19::BinomialPrice(put, n, false),
                ch19::BinomialPrice(put, n, true));
  }
  PrintValue("Black-Scholes European put", ch19::BlackScholesPrice(put));
}

void BlackScholes() {
  PrintHeading("19.5 Black-Scholes and the Greeks (S 105, r 4%, sigma 28%)");
  std::printf("  %5s %5s %8s %8s %8s %8s %9s %8s\n", "type", "K", "price",
              "delta", "gamma", "vega", "theta", "rho");
  for (OptionType type : {OptionType::kCall, OptionType::kPut}) {
    for (double k : {95.0, 105.0, 115.0}) {
      const ch19::OptionSpec s{type, 105.0, k, 0.04, 0.28, 0.5, 0.015};
      const auto g = ch19::BlackScholesGreeks(s);
      std::printf("  %5s %5.0f %8.3f %8.4f %8.5f %8.3f %9.3f %8.3f\n",
                  type == OptionType::kCall ? "call" : "put", k, g.price,
                  g.delta, g.gamma, g.vega, g.theta, g.rho);
    }
  }
}

void Volatility() {
  PrintHeading("19.5.4 Implied and historical volatility");
  const ch19::OptionSpec s{
      OptionType::kCall, 73.0, 75.0, 0.042, 0.3, 0.25, 0.0};
  for (double price : {2.50, 3.20, 4.40}) {
    std::printf("  call at %.2f -> implied vol %6.2f%%\n", price,
                100 * ch19::ImpliedVolatility(s, price).value());
  }
  const std::vector<double> closes{73.1, 74.0, 72.6, 73.9, 75.2, 74.4,
                                   75.8, 76.1, 74.9, 75.5, 77.0};
  PrintPercent("historical vol from 10 daily closes",
               ch19::HistoricalVolatility(closes, 252.0));
}

void Warrants() {
  PrintHeading("19.7 Warrants: 10m shares, 2m warrants at K 30, 3 years");
  const ch19::OptionSpec s{OptionType::kCall, 27.0, 30.0, 0.04, 0.35, 3.0, 0.0};
  PrintValue("equivalent call", ch19::BlackScholesPrice(s));
  PrintValue("warrant (dilution-adjusted)", ch19::WarrantValue(s, 1e7, 2e6));
}

}  // namespace

int main() {
  Strategies();
  Parity();
  Binomial();
  BlackScholes();
  Volatility();
  Warrants();
  return 0;
}
