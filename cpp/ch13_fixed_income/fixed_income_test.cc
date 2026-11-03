#include "cpp/ch13_fixed_income/fixed_income.h"

#include <chrono>
#include <vector>

#include "cpp/ch05_riskless_securities/cash_flows.h"
#include "gtest/gtest.h"

namespace investm::ch13 {
namespace {

using std::chrono::day;
using std::chrono::month;
using std::chrono::year;

Date D(int y, unsigned m, unsigned d) {
  return Date{year{y}, month{m}, day{d}};
}

Bond Note() {
  return Bond{100.0, 0.05, D(2030, 11, 15), 2, DayCount::kActualActual};
}

TEST(DayCountTest, Conventions) {
  EXPECT_EQ(DayCountDays(D(2026, 1, 31), D(2026, 3, 1), DayCount::kThirty360),
            31);
  EXPECT_EQ(
      DayCountDays(D(2026, 1, 31), D(2026, 3, 1), DayCount::kActualActual), 29);
  EXPECT_EQ(DayCountDays(D(2026, 5, 15), D(2026, 11, 15), DayCount::kThirty360),
            180);
  EXPECT_EQ(AddMonths(D(2027, 8, 31), -6), D(2027, 2, 28));
  EXPECT_EQ(AddMonths(D(2028, 8, 31), -6), D(2028, 2, 29));
}

TEST(FixedIncomeTest, CouponPeriodAndAccrued) {
  const Bond b = Note();
  const CouponPeriod p = SurroundingCoupons(b, D(2026, 11, 3));
  EXPECT_EQ(p.previous, D(2026, 5, 15));
  EXPECT_EQ(p.next, D(2026, 11, 15));
  EXPECT_EQ(p.remaining, 9);
  EXPECT_NEAR(AccruedInterest(b, D(2026, 11, 3)), 2.5 * 172.0 / 184.0, 1e-12);
  EXPECT_NEAR(AccruedInterest(b, D(2026, 5, 15)), 0.0, 1e-15);
}

// The direct jump to the coupon period must agree with walking back from
// maturity one period at a time, on every settlement day of several years,
// for month-end maturities (where AddMonths clamps) and each frequency.
TEST(FixedIncomeTest, SurroundingCouponsMatchesWalkingBack) {
  for (const Date maturity :
       {D(2031, 8, 31), D(2030, 11, 15), D(2032, 2, 29)}) {
    for (int frequency : {1, 2, 4, 12}) {
      const Bond bond{100.0, 0.05, maturity, frequency,
                      DayCount::kActualActual};
      const int step = 12 / frequency;
      for (std::chrono::sys_days d = std::chrono::sys_days(D(2026, 1, 1));
           Date(d) < maturity; d += std::chrono::days(3)) {
        const Date settle(d);
        int k = 0;
        Date next = maturity;
        Date previous = AddMonths(maturity, -step);
        while (previous > settle) {
          ++k;
          next = previous;
          previous = AddMonths(maturity, -step * (k + 1));
        }
        const CouponPeriod p = SurroundingCoupons(bond, settle);
        ASSERT_EQ(p.previous, previous);
        ASSERT_EQ(p.next, next);
        ASSERT_EQ(p.remaining, k + 1);
      }
    }
  }
}

TEST(FixedIncomeTest, PricesOnCouponDateMatchCashFlowValuation) {
  const Bond b = Note();
  const Date settle = D(2026, 11, 15);
  const auto flows = ch05::CouponBond(100.0, 0.05, 4.0, 2);
  EXPECT_NEAR(DirtyPrice(b, settle, 0.06),
              ch05::PresentValueAtYield(flows, 0.06, 2), 1e-10);
  EXPECT_NEAR(CleanPrice(b, settle, 0.05), 100.0, 1e-10);
}

TEST(FixedIncomeTest, CleanPriceIsSmoothAcrossCouponDate) {
  // The dirty price drops by the coupon on the payment date; the clean
  // price does not jump.
  const Bond b = Note();
  const double before = CleanPrice(b, D(2026, 11, 14), 0.06);
  const double after = CleanPrice(b, D(2026, 11, 16), 0.06);
  EXPECT_NEAR(before, after, 0.01);
  EXPECT_GT(DirtyPrice(b, D(2026, 11, 14), 0.06) -
                DirtyPrice(b, D(2026, 11, 16), 0.06),
            2.4);
}

TEST(FixedIncomeTest, YieldRoundTrip) {
  const Bond b = Note();
  const Date settle = D(2026, 11, 3);
  for (double y : {0.01, 0.045, 0.08}) {
    const auto back = YieldFromCleanPrice(b, settle, CleanPrice(b, settle, y));
    ASSERT_TRUE(back.has_value());
    EXPECT_NEAR(*back, y, 1e-10);
  }
  EXPECT_DOUBLE_EQ(PriceFrom32nds(99, 16), 99.5);
}

TEST(FixedIncomeTest, Corporate30360) {
  const Bond b{1000.0, 0.06, D(2031, 3, 31), 2, DayCount::kThirty360};
  // 30/360: Sep 30 -> Nov 3 is 33 days of a 180-day period.
  EXPECT_NEAR(AccruedInterest(b, D(2026, 11, 3)), 30.0 * 33.0 / 180.0, 1e-12);
}

TEST(CallableTest, PremiumBondYieldToWorstIsCall) {
  const std::vector<CallDate> calls{{3.0, 103.0}, {5.0, 101.0}};
  const auto ytw = YieldToWorst(100.0, 0.07, 10.0, calls, 108.0);
  ASSERT_TRUE(ytw.has_value());
  EXPECT_FALSE(ytw->is_maturity);
  const auto ytm =
      ch05::YieldToMaturity(ch05::CouponBond(100.0, 0.07, 10.0, 2), 108.0, 2);
  EXPECT_LT(ytw->yield, *ytm);
  const auto ytc = YieldToCall(
      100.0, 0.07, {ytw->horizon, ytw->horizon == 3.0 ? 103.0 : 101.0}, 108.0);
  EXPECT_NEAR(ytw->yield, *ytc, 1e-12);
  // A deep discount bond's worst case is maturity.
  const auto disc = YieldToWorst(100.0, 0.03, 10.0, calls, 85.0);
  ASSERT_TRUE(disc.has_value());
  EXPECT_TRUE(disc->is_maturity);
}

TEST(PreferredTest, Perpetuity) {
  EXPECT_DOUBLE_EQ(PerpetuityValue(5.0, 0.08), 62.5);
  EXPECT_DOUBLE_EQ(PerpetuityYield(5.0, 62.5), 0.08);
}

}  // namespace
}  // namespace investm::ch13
