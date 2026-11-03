// Worked examples for Chapter 3. All numbers are illustrative.

#include <algorithm>
#include <cstdio>
#include <vector>

#include "cpp/ch03_security_markets/call_market.h"
#include "cpp/ch03_security_markets/costs.h"
#include "cpp/core/table.h"

namespace {

using investm::PrintHeading;
using investm::PrintPercent;
using investm::ch03::CommissionSchedule;
using investm::ch03::LimitOrder;

void CallMarket() {
  PrintHeading("3.1.1 Clearing a call market");
  const std::vector<LimitOrder> book{
      {true, 30.50, 300},  {true, 30.25, 500},  {true, 30.00, 400},
      {true, 29.75, 600},  {false, 29.50, 200}, {false, 29.75, 300},
      {false, 30.00, 500}, {false, 30.25, 700},
  };
  investm::Table table({"price", "demand", "supply", "volume"});
  for (double p : {29.50, 29.75, 30.00, 30.25, 30.50}) {
    const double d = investm::ch03::DemandAt(book, p);
    const double s = investm::ch03::SupplyAt(book, p);
    table.AddRow({p, d, s, std::min(d, s)}, 2);
  }
  table.Print();
  const auto r = investm::ch03::ClearCallMarket(book);
  std::printf("  clearing price %.2f, volume %.0f, imbalance %+.0f\n", r.price,
              r.volume, r.imbalance);
}

void Commissions() {
  PrintHeading("3.7 Commissions: full-service vs. discount broker");
  const CommissionSchedule full{25.0, 0.0, 0.012, 40.0};
  const CommissionSchedule discount{4.95, 0.005, 0.0, 0.0};
  for (double shares : {100.0, 1000.0, 10000.0}) {
    std::printf("  %6.0f sh @ $40: full-service $%8.2f   discount $%7.2f\n",
                shares, full.Commission(shares, 40.0),
                discount.Commission(shares, 40.0));
  }
}

void TransactionsCosts() {
  PrintHeading("3.8 Transactions costs of a 50,000-share buy");
  const investm::ch03::Quote quote{39.98, 40.02};
  const CommissionSchedule discount{4.95, 0.005, 0.0, 0.0};
  PrintPercent("quoted relative spread", quote.RelativeSpread());
  const double impact =
      investm::ch03::SqrtPriceImpact(50000, 2'000'000, 0.02, 0.8);
  PrintPercent("estimated price impact", impact);
  PrintPercent("total one-way cost (discount broker)",
               investm::ch03::OneWayCost(50000, 40.0, quote, discount, impact));
}

}  // namespace

int main() {
  CallMarket();
  Commissions();
  TransactionsCosts();
  return 0;
}
