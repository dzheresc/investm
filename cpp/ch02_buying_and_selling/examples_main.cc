// Worked examples for Chapter 2. All numbers are illustrative.

#include <cstdio>
#include <vector>

#include "cpp/ch02_buying_and_selling/margin.h"
#include "cpp/ch02_buying_and_selling/orders.h"
#include "cpp/core/table.h"

namespace {

using investm::PrintHeading;
using investm::PrintPercent;
using investm::PrintValue;
using investm::ch02::Order;
using investm::ch02::OrderType;
using investm::ch02::Side;

void Show(const char* name, const Order& order,
          const std::vector<double>& path) {
  const auto fill = investm::ch02::Simulate(order, path);
  if (fill.has_value()) {
    std::printf("  %-36s filled at %.2f (tick %zu)\n", name, fill->price,
                fill->index);
  } else {
    std::printf("  %-36s not filled\n", name);
  }
}

void OrderTypes() {
  PrintHeading("2.3 Order types on one price path");
  const std::vector<double> path{50.0, 49.2, 48.5, 47.8,
                                 49.0, 51.5, 52.4, 50.9};
  std::printf("  path:");
  for (double p : path) std::printf(" %.2f", p);
  std::printf("\n");
  Show("market buy", {Side::kBuy, OrderType::kMarket}, path);
  Show("limit buy @ 48.00", {Side::kBuy, OrderType::kLimit, 48.0}, path);
  Show("limit buy @ 47.00", {Side::kBuy, OrderType::kLimit, 47.0}, path);
  Show("stop-loss sell @ 48.00", {Side::kSell, OrderType::kStop, 0.0, 48.0},
       path);
  Show("stop buy @ 51.00", {Side::kBuy, OrderType::kStop, 0.0, 51.0}, path);
  Show("stop-limit buy, stop 51 / limit 51.5",
       {Side::kBuy, OrderType::kStopLimit, 51.5, 51.0}, path);
}

void MarginPurchaseExample() {
  PrintHeading("2.4.1 Margin purchase: 200 shares at $60, 55% initial");
  const investm::ch02::MarginPurchase mp{200, 60.0, 0.55};
  PrintValue("cost", mp.Cost(), "$");
  PrintValue("own money", mp.EquityInvested(), "$");
  PrintValue("loan", mp.Loan(), "$");
  PrintPercent("actual margin if price falls to $45", mp.ActualMargin(45.0));
  PrintValue("margin call price (30% maintenance)", mp.MarginCallPrice(0.30),
             "$");
  PrintValue("cash to restore 55% margin at $40", mp.CashToRestore(40.0, 0.55),
             "$");
  PrintPercent("return if sold at $72, 9% loan rate",
               mp.ReturnOnEquity(72.0, 0.0, 0.09));
  PrintPercent("same trade without margin", (72.0 - 60.0) / 60.0);
  PrintPercent("return if sold at $50, 9% loan rate",
               mp.ReturnOnEquity(50.0, 0.0, 0.09));
}

void ShortSaleExample() {
  PrintHeading("2.4.2 Short sale: 300 shares at $80, 50% initial");
  const investm::ch02::ShortSale ss{300, 80.0, 0.50};
  PrintValue("proceeds held by broker", ss.Proceeds(), "$");
  PrintValue("investor deposit", ss.Deposit(), "$");
  PrintPercent("actual margin if price rises to $95", ss.ActualMargin(95.0));
  PrintValue("margin call price (30% maintenance)", ss.MarginCallPrice(0.30),
             "$");
  PrintPercent("return if covered at $64, $1 dividend",
               ss.ReturnOnEquity(64.0, 1.0));
}

void AggregationExample() {
  PrintHeading("2.4.4 Aggregation: one long and one short position");
  const investm::ch02::MarginPurchase mp{200, 60.0, 0.55};
  const investm::ch02::ShortSale ss{300, 80.0, 0.50};
  investm::ch02::Account account({{200, 60.0}, {-300, 80.0}},
                                 -mp.Loan() + ss.Proceeds() + ss.Deposit());
  PrintValue("long value", account.LongValue(), "$");
  PrintValue("short value", account.ShortValue(), "$");
  PrintValue("equity", account.Equity(), "$");
  PrintPercent("account margin", account.ActualMargin());
  account.set_positions({{200, 50.0}, {-300, 70.0}});
  PrintPercent("account margin after a broad 10-point drop",
               account.ActualMargin());
}

}  // namespace

int main() {
  OrderTypes();
  MarginPurchaseExample();
  ShortSaleExample();
  AggregationExample();
  return 0;
}
