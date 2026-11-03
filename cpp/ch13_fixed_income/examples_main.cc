// Worked examples for Chapter 13. All numbers are illustrative.

#include <chrono>
#include <cstdio>
#include <vector>

#include "cpp/ch13_fixed_income/fixed_income.h"
#include "cpp/core/table.h"

namespace {

using investm::PrintHeading;
using investm::PrintPercent;
using investm::PrintValue;
namespace ch13 = investm::ch13;

ch13::Date D(int y, unsigned m, unsigned d) {
  return ch13::Date{std::chrono::year{y}, std::chrono::month{m},
                    std::chrono::day{d}};
}

void Treasury() {
  PrintHeading("13.3 A Treasury note between coupon dates");
  const ch13::Bond note{100.0, 0.0425, D(2034, 2, 15), 2,
                        ch13::DayCount::kActualActual};
  const ch13::Date settle = D(2026, 11, 3);
  const auto p = ch13::SurroundingCoupons(note, settle);
  std::printf("  4.25%% note due 2034-02-15, settling 2026-11-03\n");
  std::printf("  last coupon %d-%02u-%02u, next %d-%02u-%02u, %d to go\n",
              static_cast<int>(p.previous.year()),
              static_cast<unsigned>(p.previous.month()),
              static_cast<unsigned>(p.previous.day()),
              static_cast<int>(p.next.year()),
              static_cast<unsigned>(p.next.month()),
              static_cast<unsigned>(p.next.day()), p.remaining);
  const double quoted = ch13::PriceFrom32nds(98, 11);
  PrintValue("quoted price 98-11", quoted);
  PrintValue("accrued interest", ch13::AccruedInterest(note, settle));
  PrintValue("invoice (dirty) price",
             quoted + ch13::AccruedInterest(note, settle));
  PrintPercent("yield to maturity (bond-equivalent)",
               ch13::YieldFromCleanPrice(note, settle, quoted).value());

  std::printf("\n  clean vs dirty price at 4.5%% through a coupon date:\n");
  for (unsigned d : {10u, 13u, 14u, 15u, 16u, 17u}) {
    const ch13::Date s = D(2027, 2, d);
    std::printf("    2027-02-%02u  clean %8.4f  dirty %8.4f\n", d,
                ch13::CleanPrice(note, s, 0.045),
                ch13::DirtyPrice(note, s, 0.045));
  }
}

void Corporate() {
  PrintHeading("13.5 A corporate bond on a 30/360 basis");
  const ch13::Bond corp{1000.0, 0.0575, D(2033, 6, 30), 2,
                        ch13::DayCount::kThirty360};
  const ch13::Date settle = D(2026, 11, 3);
  PrintValue("accrued per $1000", ch13::AccruedInterest(corp, settle));
  PrintValue("clean price at 6.40%", ch13::CleanPrice(corp, settle, 0.064));
}

void Callable() {
  PrintHeading("13.6 Callable bonds: yield-to-call and yield-to-worst");
  const std::vector<ch13::CallDate> calls{
      {2.0, 104.0}, {4.0, 102.0}, {6.0, 100.0}};
  std::printf(
      "  6.5%% 12-year bond, call schedule 104 (yr 2), 102 (yr 4), "
      "100 (yr 6)\n");
  std::printf("  %8s %10s %12s\n", "price", "YTW", "worst case");
  for (double price : {92.0, 100.0, 106.0, 112.0}) {
    const auto w = ch13::YieldToWorst(100.0, 0.065, 12.0, calls, price);
    std::printf("  %8.2f %9.3f%% %9.0f yr%s\n", price, 100 * w->yield,
                w->horizon, w->is_maturity ? " (maturity)" : " (call)");
  }
}

void Preferred() {
  PrintHeading("13.8 Preferred stock");
  PrintValue("value of $4.40 dividend at 6.5%",
             ch13::PerpetuityValue(4.40, 0.065));
  PrintPercent("yield at a price of $58", ch13::PerpetuityYield(4.40, 58.0));
}

}  // namespace

int main() {
  Treasury();
  Corporate();
  Callable();
  Preferred();
  return 0;
}
