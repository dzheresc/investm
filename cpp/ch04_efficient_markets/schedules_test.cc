#include "cpp/ch04_efficient_markets/schedules.h"

#include <vector>

#include "gtest/gtest.h"

namespace investm::ch04 {
namespace {

TEST(SchedulesTest, EquilibriumClearsAndFollowsValuations) {
  std::vector<Investor> investors{
      {48, 1000, 800}, {52, 400, 900}, {55, 200, 1200}, {45, 1500, 500}};
  const auto p = EquilibriumPrice(investors);
  ASSERT_TRUE(p.has_value());
  EXPECT_GT(*p, 45.0);
  EXPECT_LT(*p, 55.0);
  EXPECT_NEAR(DemandToBuy(investors, *p, 5.0), SupplyToSell(investors, *p, 5.0),
              1e-6);
  for (Investor& i : investors) i.value += 2.0;
  EXPECT_GT(*EquilibriumPrice(investors), *p);
}

TEST(SchedulesTest, UnanimousValuationHasNoEquilibrium) {
  const std::vector<Investor> investors{{50, 100, 100}, {50, 100, 100}};
  EXPECT_FALSE(EquilibriumPrice(investors).has_value());
}

TEST(SchedulesTest, ArcElasticity) {
  EXPECT_NEAR(ArcElasticity(10, 100, 11, 80), (-20.0 / 90.0) / (1.0 / 10.5),
              1e-12);
}

}  // namespace
}  // namespace investm::ch04
