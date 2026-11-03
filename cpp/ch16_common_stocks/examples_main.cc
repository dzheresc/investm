// Worked examples for Chapter 16. All numbers are illustrative.

#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <vector>

#include "cpp/ch16_common_stocks/betas.h"
#include "cpp/ch16_common_stocks/corporate.h"
#include "cpp/core/table.h"

namespace {

using investm::PrintHeading;
using investm::PrintValue;
namespace ch16 = investm::ch16;

void Voting() {
  PrintHeading("16.1.3 Cumulative vs majority voting (2.4m shares, 7 seats)");
  const std::int64_t shares = 2400000;
  for (int seats = 1; seats <= 4; ++seats) {
    const std::int64_t need =
        ch16::CumulativeVotingSharesNeeded(shares, 7, seats);
    std::printf(
        "  seats %d: cumulative needs %9" PRId64 " shares (%5.2f%%)\n", seats,
        need, 100.0 * static_cast<double>(need) / static_cast<double>(shares));
  }
  std::printf("  majority voting: any seat needs %" PRId64 " shares\n",
              ch16::MajorityVotingSharesNeeded(shares));
}

void Splits() {
  PrintHeading("16.3 Splits and stock dividends");
  PrintValue("price after 3-for-2 split of $87",
             ch16::PriceAfterSplit(87, 1.5));
  PrintValue(
      "price after 15% stock dividend on $46",
      ch16::PriceAfterSplit(46, ch16::SplitRatioFromStockDividend(0.15)));
  const std::vector<double> prices{120.0, 123.0, 41.5, 42.2, 21.4, 21.9};
  const std::vector<double> ratios{1.0, 1.0, 3.0, 1.0, 2.0, 1.0};
  const auto adj = ch16::SplitAdjustPrices(prices, ratios);
  std::printf("  %4s %9s %6s %9s %9s\n", "day", "raw", "split", "adjusted",
              "return");
  for (std::size_t t = 0; t < prices.size(); ++t) {
    std::printf("  %4zu %9.2f %6.1f %9.3f", t, prices[t], ratios[t], adj[t]);
    if (t > 0) std::printf(" %8.2f%%", 100 * (adj[t] / adj[t - 1] - 1));
    std::printf("\n");
  }
}

void Rights() {
  PrintHeading("16.4 A rights offering: 5 rights + $32 buy one new share");
  const ch16::RightsTerms terms{5, 32.0};
  const double cum = 41.0;
  const double ex = ch16::ExRightsPrice(cum, terms);
  PrintValue("rights-on price", cum);
  PrintValue("theoretical ex-rights price", ex);
  PrintValue("value of a right (rights-on formula)",
             ch16::RightValueCum(cum, terms));
  PrintValue("value of a right (ex-rights formula)",
             ch16::RightValueEx(ex, terms));
}

void Betas() {
  PrintHeading("16.7 Estimating and adjusting betas");
  const std::vector<double> market{0.031, -0.012, 0.024, 0.008, -0.035, 0.042,
                                   0.015, -0.021, 0.019, 0.027, -0.008, 0.011};
  const std::vector<double> stock{0.048, -0.025, 0.029, 0.021, -0.061, 0.066,
                                  0.012, -0.027, 0.034, 0.031, -0.019, 0.024};
  const auto fit = ch16::EstimateBeta(stock, market);
  std::printf("  raw beta %.3f (se %.3f), alpha %.4f, R^2 %.3f\n", fit.beta,
              fit.beta_std_error, fit.alpha, fit.r_squared);
  PrintValue("adjusted beta (2/3 raw + 1/3 x 1.0)", ch16::ShrinkBeta(fit.beta));
  PrintValue("Vasicek beta (prior 1.0, sd 0.35)",
             ch16::VasicekBeta(fit.beta, fit.beta_std_error, 1.0, 0.35));

  const std::vector<double> earlier{0.45, 0.70, 0.95, 1.10, 1.35, 1.60, 1.90};
  const std::vector<double> later{0.62, 0.81, 0.97, 1.06, 1.22, 1.39, 1.57};
  const auto p = ch16::BetaPersistence(earlier, later);
  std::printf("  later = %.3f + %.3f x earlier: betas regress toward 1\n",
              p.alpha, p.beta);

  PrintHeading("16.7 Leverage and beta (25% tax)");
  const double asset_beta = ch16::UnleveredBeta(1.30, 0.60, 0.25);
  PrintValue("asset beta of a 1.30 equity beta at D/E 0.60", asset_beta);
  for (double de : {0.0, 0.3, 0.6, 1.0, 1.5}) {
    std::printf("  D/E %.1f -> equity beta %.3f\n", de,
                ch16::LeveredBeta(asset_beta, de, 0.25));
  }
}

}  // namespace

int main() {
  Voting();
  Splits();
  Rights();
  Betas();
  return 0;
}
