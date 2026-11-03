// Google Benchmark comparison of investm and QuantLib on the same inputs,
// C++ against C++. Each pair of benchmarks computes one quantity with
// matched settings and reports the result as the "value" counter (and, for
// a few cases, an "error" counter against a reference), so the JSON output
// carries accuracy next to time. Build with -DINVESTM_BUILD_COMPARE=ON
// (needs QuantLib and Google Benchmark) and run:
//   ./build/compare/cpp/compare/investm_compare_quantlib
//       --benchmark_out=compare.json --benchmark_out_format=json
// docs/comparison.md has the results and the reference values.

#include <benchmark/benchmark.h>

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstddef>
#include <memory>
#include <optional>
#include <random>
#include <span>
#include <vector>

#include "cpp/ch05_riskless_securities/cash_flows.h"
#include "cpp/ch05_riskless_securities/term_structure.h"
#include "cpp/ch06_portfolio_selection/risk_measures.h"
#include "cpp/ch13_fixed_income/fixed_income.h"
#include "cpp/ch14_bond_analysis/duration.h"
#include "cpp/ch19_options/payoffs.h"
#include "cpp/ch19_options/pricing.h"
#include "cpp/core/matrix.h"
#include "cpp/core/stats.h"
#include "ql/quantlib.hpp"

namespace investm::compare {
namespace {

namespace ql = QuantLib;

// QuantLib works with dates. 30/360 on the 15th of the month makes whole
// months exact fractions of a year, so T = 5/12, 0.5, 1 and 10 match
// investm's year fractions exactly.
const ql::Date kToday(15, ql::January, 2026);
// A function rather than a global: day counters hold a shared_ptr, and
// globals must be trivially destructible.
ql::DayCounter DayCount() { return ql::Thirty360(ql::Thirty360::USA); }

void SetToday() { ql::Settings::instance().evaluationDate() = kToday; }

ql::Handle<ql::YieldTermStructure> Flat(double rate) {
  return ql::Handle<ql::YieldTermStructure>(
      ql::ext::make_shared<ql::FlatForward>(kToday, rate, DayCount(),
                                            ql::Continuous));
}

ql::ext::shared_ptr<ql::GeneralizedBlackScholesProcess> Process(double spot,
                                                                double rate,
                                                                double vol) {
  return ql::ext::make_shared<ql::BlackScholesMertonProcess>(
      ql::Handle<ql::Quote>(ql::ext::make_shared<ql::SimpleQuote>(spot)),
      Flat(0.0), Flat(rate),
      ql::Handle<ql::BlackVolTermStructure>(
          ql::ext::make_shared<ql::BlackConstantVol>(kToday, ql::NullCalendar(),
                                                     vol, DayCount())));
}

// Prices `option` with `engine` on every iteration. Setting the engine
// marks the instrument as needing recalculation.
void RunQuantLib(benchmark::State& state, ql::Instrument& option,
                 const ql::ext::shared_ptr<ql::PricingEngine>& engine) {
  for (auto _ : state) {
    option.setPricingEngine(engine);
    benchmark::DoNotOptimize(option.NPV());
  }
  option.setPricingEngine(engine);
  state.counters["value"] = option.NPV();
}

// Times `compute` and reports one more call's result as the "value"
// counter, taken outside the timed loop.
double Value(double x) { return x; }
double Value(const std::optional<double>& x) { return x.value_or(NAN); }

template <typename F>
void Run(benchmark::State& state, F compute) {
  for (auto _ : state) {
    auto result = compute();
    benchmark::DoNotOptimize(result);
  }
  state.counters["value"] = Value(compute());
}

// ---------------------------------------------------------------------------
// Chapter 19: Black-Scholes, Greeks, implied volatility, binomial trees.

constexpr ch19::OptionType kCall = ch19::OptionType::kCall;

ch19::OptionSpec Spec(ch19::OptionType type, double spot, double strike,
                      double rate, double vol, double years) {
  ch19::OptionSpec s;
  s.type = type;
  s.spot = spot;
  s.strike = strike;
  s.rate = rate;
  s.volatility = vol;
  s.years = years;
  return s;
}

// BSM call, S = 42, K = 40, r = 10%, sigma = 20%, T = 0.5.
const ch19::OptionSpec kBsmCall = Spec(kCall, 42.0, 40.0, 0.1, 0.2, 0.5);

struct BlackInputs {
  double df;
  double forward;
  double std_dev;
};

BlackInputs Black(const ch19::OptionSpec& s) {
  const double df = std::exp(-s.rate * s.years);
  return {df, s.spot * std::exp((s.rate - s.dividend_yield) * s.years),
          s.volatility * std::sqrt(s.years)};
}

void BmBsmInvestm(benchmark::State& state) {
  Run(state, [] { return ch19::BlackScholesPrice(kBsmCall); });
}
BENCHMARK(BmBsmInvestm);

void BmBsmQuantLib(benchmark::State& state) {
  const BlackInputs b = Black(kBsmCall);
  Run(state, [&] {
    return ql::blackFormula(ql::Option::Call, kBsmCall.strike, b.forward,
                            b.std_dev, b.df);
  });
}
BENCHMARK(BmBsmQuantLib);

// The same price through QuantLib's instrument and engine objects.
void BmBsmQuantLibEngine(benchmark::State& state) {
  SetToday();
  ql::VanillaOption option(
      ql::ext::make_shared<ql::PlainVanillaPayoff>(ql::Option::Call, 40.0),
      ql::ext::make_shared<ql::EuropeanExercise>(kToday +
                                                 ql::Period(6, ql::Months)));
  auto engine =
      ql::ext::make_shared<ql::AnalyticEuropeanEngine>(Process(42.0, 0.1, 0.2));
  RunQuantLib(state, option, engine);
}
BENCHMARK(BmBsmQuantLibEngine);

void BmGreeksInvestm(benchmark::State& state) {
  for (auto _ : state) {
    benchmark::DoNotOptimize(ch19::BlackScholesGreeks(kBsmCall));
  }
  const ch19::Greeks g = ch19::BlackScholesGreeks(kBsmCall);
  state.counters["value"] = g.price;
  state.counters["delta"] = g.delta;
  state.counters["gamma"] = g.gamma;
  state.counters["vega"] = g.vega;
  state.counters["theta"] = g.theta;
  state.counters["rho"] = g.rho;
}
BENCHMARK(BmGreeksInvestm);

// BlackCalculator gives all six from one object, as investm's struct does.
void BmGreeksQuantLib(benchmark::State& state) {
  const BlackInputs b = Black(kBsmCall);
  const double s = kBsmCall.spot;
  const double t = kBsmCall.years;
  auto all = [&] {
    ql::BlackCalculator c(ql::Option::Call, kBsmCall.strike, b.forward,
                          b.std_dev, b.df);
    return ch19::Greeks{c.value(), c.delta(s),    c.gamma(s),
                        c.vega(t), c.theta(s, t), c.rho(t)};
  };
  for (auto _ : state) benchmark::DoNotOptimize(all());
  const ch19::Greeks g = all();
  state.counters["value"] = g.price;
  state.counters["delta"] = g.delta;
  state.counters["gamma"] = g.gamma;
  state.counters["vega"] = g.vega;
  state.counters["theta"] = g.theta;
  state.counters["rho"] = g.rho;
}
BENCHMARK(BmGreeksQuantLib);

// Implied volatility of a call at 1.875, S = 21, K = 20, r = 10%, T = 0.25.
const ch19::OptionSpec kIvCall = Spec(kCall, 21.0, 20.0, 0.1, 0.2, 0.25);
constexpr double kIvPrice = 1.875;

void BmImpliedVolInvestm(benchmark::State& state) {
  Run(state, [] { return ch19::ImpliedVolatility(kIvCall, kIvPrice); });
}
BENCHMARK(BmImpliedVolInvestm);

// arg 0: QuantLib's default accuracy (1e-6 on the standard deviation);
// arg 1: 1e-12, to match investm's Brent tolerance.
void BmImpliedVolQuantLib(benchmark::State& state) {
  const BlackInputs b = Black(kIvCall);
  const double accuracy = state.range(0) == 0 ? 1e-6 : 1e-12;
  const double sqrt_t = std::sqrt(kIvCall.years);
  Run(state, [&] {
    return ql::blackFormulaImpliedStdDev(ql::Option::Call, kIvCall.strike,
                                         b.forward, kIvPrice, b.df, 0.0,
                                         ql::Null<ql::Real>(), accuracy) /
           sqrt_t;
  });
}
BENCHMARK(BmImpliedVolQuantLib)->Arg(0)->Arg(1);

void CrrArgs(benchmark::internal::Benchmark* b) {
  b->Args({100, 1});
  b->Args({1000, 1});
  b->Args({1000, 0});
  b->Unit(benchmark::kMicrosecond);
}

// CRR tree, S = K = 50, r = 10%, sigma = 40%, T = 5/12. arg 0 is the number
// of steps, arg 1 is 1 for an American put and 0 for a European one.
void BmCrrInvestm(benchmark::State& state) {
  const ch19::OptionSpec put =
      Spec(ch19::OptionType::kPut, 50.0, 50.0, 0.1, 0.4, 5.0 / 12.0);
  const int steps = static_cast<int>(state.range(0));
  const bool american = state.range(1) == 1;
  Run(state, [&] { return ch19::BinomialPrice(put, steps, american); });
}
BENCHMARK(BmCrrInvestm)->Apply(CrrArgs);

void BmCrrQuantLib(benchmark::State& state) {
  SetToday();
  const ql::Date expiry = kToday + ql::Period(5, ql::Months);
  ql::ext::shared_ptr<ql::Exercise> exercise;
  if (state.range(1) == 1) {
    exercise = ql::ext::make_shared<ql::AmericanExercise>(kToday, expiry);
  } else {
    exercise = ql::ext::make_shared<ql::EuropeanExercise>(expiry);
  }
  ql::VanillaOption option(
      ql::ext::make_shared<ql::PlainVanillaPayoff>(ql::Option::Put, 50.0),
      exercise);
  auto engine =
      ql::ext::make_shared<ql::BinomialVanillaEngine<ql::CoxRossRubinstein>>(
          Process(50.0, 0.1, 0.4), static_cast<ql::Size>(state.range(0)));
  RunQuantLib(state, option, engine);
}
BENCHMARK(BmCrrQuantLib)->Apply(CrrArgs);

// ---------------------------------------------------------------------------
// Chapters 6 and 19: the normal distribution. Each call evaluates one point
// of a fixed grid, cycling through it; "error" is the largest error over the
// grid against a long double reference.

std::vector<double> Grid(double lo, double hi, int n) {
  std::vector<double> x(static_cast<std::size_t>(n));
  for (int i = 0; i < n; ++i) {
    x[static_cast<std::size_t>(i)] = lo + (hi - lo) * i / (n - 1);
  }
  return x;
}

long double NormCdfReference(long double x) {
  return 0.5L * std::erfc(-x / std::sqrt(2.0L));
}

// Relative error for the CDF (it spans 1e-15 to 1 on [-8, 8]).
template <typename F>
void RunNormCdf(benchmark::State& state, F cdf) {
  const std::vector<double> x = Grid(-8.0, 8.0, 1001);
  std::size_t i = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize(cdf(x[i]));
    if (++i == x.size()) i = 0;
  }
  double worst = 0.0;
  double worst_abs = 0.0;
  for (double xi : x) {
    const long double ref = NormCdfReference(xi);
    const long double diff = std::fabs(cdf(xi) - ref);
    worst = std::max(worst, static_cast<double>(diff / ref));
    worst_abs = std::max(worst_abs, static_cast<double>(diff));
  }
  state.counters["error"] = worst;
  state.counters["abs_error"] = worst_abs;
}

void BmNormCdfInvestm(benchmark::State& state) {
  RunNormCdf(state, [](double x) { return stats::NormCdf(x); });
}
BENCHMARK(BmNormCdfInvestm);

void BmNormCdfQuantLib(benchmark::State& state) {
  const ql::CumulativeNormalDistribution n;
  RunNormCdf(state, [&](double x) { return n(x); });
}
BENCHMARK(BmNormCdfQuantLib);

// Probabilities from 1e-10 to 0.5 on a log scale, the tail used for value
// at risk. (Above 0.5, p itself carries a rounding error of 1e-16 that both
// libraries inherit, so the lower half is the meaningful test.) The
// reference is Newton-polished in long double. "error" is the largest
// absolute error in z.
std::vector<double> ProbabilityGrid() {
  std::vector<double> p;
  for (int i = 0; i <= 500; ++i) p.push_back(std::pow(10.0, -10.0 + i / 50.0));
  p.back() = 0.5;
  return p;
}

long double NormInvReference(double p, double guess) {
  long double z = guess;
  for (int k = 0; k < 4; ++k) {
    const long double pdf =
        std::exp(-0.5L * z * z) / std::sqrt(2.0L * 3.14159265358979323846L);
    z -= (NormCdfReference(z) - p) / pdf;
  }
  return z;
}

template <typename F>
void RunNormInv(benchmark::State& state, F inv) {
  const std::vector<double> p = ProbabilityGrid();
  std::size_t i = 0;
  for (auto _ : state) {
    benchmark::DoNotOptimize(inv(p[i]));
    if (++i == p.size()) i = 0;
  }
  double worst = 0.0;
  for (double pi : p) {
    const double z = inv(pi);
    worst = std::max(
        worst, static_cast<double>(std::fabs(z - NormInvReference(pi, z))));
  }
  state.counters["error"] = worst;
}

void BmNormInvInvestm(benchmark::State& state) {
  RunNormInv(state, [](double p) { return ch06::NormInv(p); });
}
BENCHMARK(BmNormInvInvestm);

void BmNormInvQuantLib(benchmark::State& state) {
  const ql::InverseCumulativeNormal n;
  RunNormInv(state, [&](double p) { return n(p); });
}
BENCHMARK(BmNormInvQuantLib);

// ---------------------------------------------------------------------------
// Chapters 5 and 14: a 10-year 8% semiannual bond at a 9% yield, priced on a
// coupon date. Price, yield to maturity, duration and convexity.

constexpr double kFace = 100.0;
constexpr double kCoupon = 0.08;
constexpr double kYield = 0.09;

const ch05::CashFlows& InvestmBond() {
  static const ch05::CashFlows flows = ch05::CouponBond(kFace, kCoupon, 10, 2);
  return flows;
}

ql::FixedRateBond QuantLibBond() {
  SetToday();
  const ql::Schedule schedule(kToday, kToday + ql::Period(10, ql::Years),
                              ql::Period(ql::Semiannual), ql::NullCalendar(),
                              ql::Unadjusted, ql::Unadjusted,
                              ql::DateGeneration::Backward, false);
  return ql::FixedRateBond(0, kFace, schedule, {kCoupon}, DayCount(),
                           ql::Unadjusted);
}

ql::InterestRate QuantLibYield(double y) {
  return ql::InterestRate(y, DayCount(), ql::Compounded, ql::Semiannual);
}

void BmBondPriceInvestm(benchmark::State& state) {
  Run(state,
      [] { return ch05::PresentValueAtYield(InvestmBond(), kYield, 2); });
}
BENCHMARK(BmBondPriceInvestm);

void BmBondPriceQuantLib(benchmark::State& state) {
  const ql::FixedRateBond bond = QuantLibBond();
  const ql::InterestRate y = QuantLibYield(kYield);
  Run(state, [&] { return ql::BondFunctions::cleanPrice(bond, y, kToday); });
}
BENCHMARK(BmBondPriceQuantLib);

double QuotedPrice() {
  return ch05::PresentValueAtYield(InvestmBond(), kYield, 2);
}

void BmBondYieldInvestm(benchmark::State& state) {
  const double price = QuotedPrice();
  Run(state, [&] { return ch05::YieldToMaturity(InvestmBond(), price, 2); });
  state.counters["error"] = state.counters["value"].value - kYield;
}
BENCHMARK(BmBondYieldInvestm);

// arg 0: QuantLib's default accuracy, 1e-10; arg 1: 1e-12, investm's Brent
// tolerance.
void BmBondYieldQuantLib(benchmark::State& state) {
  const ql::FixedRateBond bond = QuantLibBond();
  const double price = QuotedPrice();
  const double accuracy = state.range(0) == 0 ? 1e-10 : 1e-12;
  const ql::DayCounter dc = DayCount();
  Run(state, [&] {
    return ql::BondFunctions::yield(bond, price, dc, ql::Compounded,
                                    ql::Semiannual, kToday, accuracy);
  });
  state.counters["error"] = state.counters["value"].value - kYield;
}
BENCHMARK(BmBondYieldQuantLib)->Arg(0)->Arg(1);

void BmDurationInvestm(benchmark::State& state) {
  auto all = [] {
    const ch05::CashFlows& f = InvestmBond();
    return std::array<double, 3>{ch14::MacaulayDuration(f, kYield, 2),
                                 ch14::ModifiedDuration(f, kYield, 2),
                                 ch14::Convexity(f, kYield, 2)};
  };
  for (auto _ : state) benchmark::DoNotOptimize(all());
  const auto r = all();
  state.counters["macaulay"] = r[0];
  state.counters["modified"] = r[1];
  state.counters["convexity"] = r[2];
}
BENCHMARK(BmDurationInvestm);

void BmDurationQuantLib(benchmark::State& state) {
  const ql::FixedRateBond bond = QuantLibBond();
  const ql::InterestRate y = QuantLibYield(kYield);
  auto all = [&] {
    using ql::BondFunctions;
    return std::array<double, 3>{
        BondFunctions::duration(bond, y, ql::Duration::Macaulay, kToday),
        BondFunctions::duration(bond, y, ql::Duration::Modified, kToday),
        BondFunctions::convexity(bond, y, kToday)};
  };
  for (auto _ : state) benchmark::DoNotOptimize(all());
  const auto r = all();
  state.counters["macaulay"] = r[0];
  state.counters["modified"] = r[1];
  state.counters["convexity"] = r[2];
}
BENCHMARK(BmDurationQuantLib);

// ---------------------------------------------------------------------------
// Chapter 13: a 7% semiannual Treasury maturing 15 November 2036, settled
// between coupon dates on 1 March 2027, actual/actual. Accrued interest and
// clean price at 6.5%.

constexpr double kTreasuryCoupon = 0.07;
constexpr double kTreasuryYield = 0.065;

ch13::Date Ymd(int y, unsigned m, unsigned d) {
  return ch13::Date(std::chrono::year(y), std::chrono::month(m),
                    std::chrono::day(d));
}

ch13::Bond InvestmTreasury() {
  ch13::Bond bond;
  bond.face = kFace;
  bond.coupon_rate = kTreasuryCoupon;
  bond.maturity = Ymd(2036, 11, 15);
  bond.frequency = 2;
  bond.day_count = ch13::DayCount::kActualActual;
  return bond;
}

const ch13::Date kSettle = Ymd(2027, 3, 1);
const ql::Date kQlSettle(1, ql::March, 2027);

void BmTreasuryInvestm(benchmark::State& state) {
  const ch13::Bond bond = InvestmTreasury();
  auto both = [&] {
    return std::array<double, 2>{
        ch13::AccruedInterest(bond, kSettle),
        ch13::CleanPrice(bond, kSettle, kTreasuryYield)};
  };
  for (auto _ : state) benchmark::DoNotOptimize(both());
  const auto r = both();
  state.counters["accrued"] = r[0];
  state.counters["value"] = r[1];
}
BENCHMARK(BmTreasuryInvestm);

void BmTreasuryQuantLib(benchmark::State& state) {
  ql::Settings::instance().evaluationDate() = kQlSettle;
  const ql::Schedule schedule(
      ql::Date(15, ql::November, 2026), ql::Date(15, ql::November, 2036),
      ql::Period(ql::Semiannual), ql::NullCalendar(), ql::Unadjusted,
      ql::Unadjusted, ql::DateGeneration::Backward, false);
  const ql::ActualActual dc(ql::ActualActual::ISMA, schedule);
  const ql::FixedRateBond bond(0, kFace, schedule, {kTreasuryCoupon}, dc,
                               ql::Unadjusted);
  const ql::InterestRate y(kTreasuryYield, dc, ql::Compounded, ql::Semiannual);
  auto both = [&] {
    return std::array<double, 2>{
        bond.accruedAmount(kQlSettle),
        ql::BondFunctions::cleanPrice(bond, y, kQlSettle)};
  };
  for (auto _ : state) benchmark::DoNotOptimize(both());
  const auto r = both();
  state.counters["accrued"] = r[0];
  state.counters["value"] = r[1];
}
BENCHMARK(BmTreasuryQuantLib);

// ---------------------------------------------------------------------------
// Chapter 5: bootstrapping spot rates from 30 annual-coupon bonds whose
// prices come from a known curve, s(t) = 3% + 2% (1 - exp(-t / 5)). "error"
// is the largest error in a discount factor at years 1..30.

constexpr int kBootstrapYears = 30;

double TrueDiscount(int t) {
  const double s = 0.03 + 0.02 * (1.0 - std::exp(-t / 5.0));
  return std::pow(1.0 + s, -t);
}

std::vector<ch05::BondQuote> BootstrapQuotes() {
  std::vector<ch05::BondQuote> quotes;
  double annuity = 0.0;
  for (int n = 1; n <= kBootstrapYears; ++n) {
    const double coupon = 0.04 + 0.0005 * n;
    annuity += TrueDiscount(n);
    quotes.push_back(
        {kFace, coupon, kFace * (coupon * annuity + TrueDiscount(n))});
  }
  return quotes;
}

void BmBootstrapInvestm(benchmark::State& state) {
  const std::vector<ch05::BondQuote> quotes = BootstrapQuotes();
  for (auto _ : state) benchmark::DoNotOptimize(ch05::Bootstrap(quotes));
  const ch05::SpotCurve curve = *ch05::Bootstrap(quotes);
  double worst = 0.0;
  for (int t = 1; t <= kBootstrapYears; ++t) {
    worst =
        std::max(worst, std::fabs(curve.DiscountFactor(t) - TrueDiscount(t)));
  }
  state.counters["error"] = worst;
}
BENCHMARK(BmBootstrapInvestm);

// Piecewise log-linear discount curve on FixedRateBondHelpers, the standard
// QuantLib bond bootstrap. Each iteration builds a new curve.
void BmBootstrapQuantLib(benchmark::State& state) {
  SetToday();
  std::vector<ql::ext::shared_ptr<ql::RateHelper>> helpers;
  for (const ch05::BondQuote& q : BootstrapQuotes()) {
    const int n = static_cast<int>(helpers.size()) + 1;
    const ql::Schedule schedule(kToday, kToday + ql::Period(n, ql::Years),
                                ql::Period(ql::Annual), ql::NullCalendar(),
                                ql::Unadjusted, ql::Unadjusted,
                                ql::DateGeneration::Backward, false);
    helpers.push_back(ql::ext::make_shared<ql::FixedRateBondHelper>(
        ql::Handle<ql::Quote>(ql::ext::make_shared<ql::SimpleQuote>(q.price)),
        0, kFace, schedule, std::vector<ql::Rate>{q.coupon_rate}, DayCount(),
        ql::Unadjusted));
  }
  auto build = [&] {
    return ql::ext::make_shared<
        ql::PiecewiseYieldCurve<ql::Discount, ql::LogLinear>>(kToday, helpers,
                                                              DayCount());
  };
  for (auto _ : state) {
    auto curve = build();
    benchmark::DoNotOptimize(curve->discount(1.0 * kBootstrapYears));
  }
  auto curve = build();
  double worst = 0.0;
  for (int t = 1; t <= kBootstrapYears; ++t) {
    worst =
        std::max(worst, std::fabs(curve->discount(1.0 * t) - TrueDiscount(t)));
  }
  state.counters["error"] = worst;
}
BENCHMARK(BmBootstrapQuantLib)->Unit(benchmark::kMicrosecond);

// ---------------------------------------------------------------------------
// Chapters 7 and 10: linear algebra on an n x n sample covariance matrix of
// 3-factor returns (n + 60 observations). "error" is the largest residual
// |A v - lambda v| for the eigen-decomposition, |L L' - A| for Cholesky and
// |A A^-1 - I| for the inverse.

Matrix Covariance(std::size_t n) {
  std::mt19937 rng(7);
  std::normal_distribution<double> z(0.0, 1.0);
  const std::size_t t = n + 60;
  std::vector<std::array<double, 3>> b(n);
  for (auto& bi : b) {
    for (double& x : bi) x = 0.5 + 0.5 * z(rng);
  }
  Matrix r(t, n);
  for (std::size_t i = 0; i < t; ++i) {
    const double f[3] = {0.04 * z(rng), 0.02 * z(rng), 0.015 * z(rng)};
    for (std::size_t j = 0; j < n; ++j) {
      r(i, j) =
          b[j][0] * f[0] + b[j][1] * f[1] + b[j][2] * f[2] + 0.05 * z(rng);
    }
  }
  return CovarianceMatrix(r);
}

ql::Matrix ToQuantLib(const Matrix& a) {
  ql::Matrix m(a.rows(), a.cols());
  for (std::size_t i = 0; i < a.rows(); ++i) {
    for (std::size_t j = 0; j < a.cols(); ++j) m[i][j] = a(i, j);
  }
  return m;
}

// Largest |A v_k - lambda_k v_k| over all eigenpairs; get(i, k) reads
// component i of eigenvector k.
template <typename Get>
double EigenResidual(const Matrix& a, std::span<const double> values, Get get) {
  const std::size_t n = a.rows();
  double worst = 0.0;
  for (std::size_t k = 0; k < n; ++k) {
    for (std::size_t i = 0; i < n; ++i) {
      double av = 0.0;
      for (std::size_t j = 0; j < n; ++j) av += a(i, j) * get(j, k);
      worst = std::max(worst, std::fabs(av - values[k] * get(i, k)));
    }
  }
  return worst;
}

void BmEigenInvestm(benchmark::State& state) {
  const Matrix a = Covariance(static_cast<std::size_t>(state.range(0)));
  for (auto _ : state) benchmark::DoNotOptimize(EigenSymmetric(a));
  const SymmetricEigen e = EigenSymmetric(a);
  state.counters["value"] = e.values.front();
  state.counters["error"] = EigenResidual(
      a, e.values,
      [&](std::size_t i, std::size_t k) { return e.vectors(i, k); });
}
BENCHMARK(BmEigenInvestm)->Arg(20)->Arg(100)->Unit(benchmark::kMicrosecond);

void BmEigenQuantLib(benchmark::State& state) {
  const Matrix a = Covariance(static_cast<std::size_t>(state.range(0)));
  const ql::Matrix m = ToQuantLib(a);
  for (auto _ : state) {
    ql::SymmetricSchurDecomposition e(m);
    benchmark::DoNotOptimize(e.eigenvalues()[0]);
  }
  const ql::SymmetricSchurDecomposition e(m);
  const ql::Array& values = e.eigenvalues();
  const Vector v(values.begin(), values.end());
  state.counters["value"] = v.front();
  state.counters["error"] = EigenResidual(
      a, v,
      [&](std::size_t i, std::size_t k) { return e.eigenvectors()[i][k]; });
}
BENCHMARK(BmEigenQuantLib)->Arg(20)->Arg(100)->Unit(benchmark::kMicrosecond);

template <typename Get>
double CholeskyResidual(const Matrix& a, Get l) {
  const std::size_t n = a.rows();
  double worst = 0.0;
  for (std::size_t i = 0; i < n; ++i) {
    for (std::size_t j = 0; j < n; ++j) {
      double s = 0.0;
      for (std::size_t k = 0; k < n; ++k) s += l(i, k) * l(j, k);
      worst = std::max(worst, std::fabs(s - a(i, j)));
    }
  }
  return worst;
}

void BmCholeskyInvestm(benchmark::State& state) {
  const Matrix a = Covariance(100);
  for (auto _ : state) benchmark::DoNotOptimize(Cholesky(a));
  const Matrix l = *Cholesky(a);
  state.counters["error"] = CholeskyResidual(
      a, [&](std::size_t i, std::size_t k) { return l(i, k); });
}
BENCHMARK(BmCholeskyInvestm)->Unit(benchmark::kMicrosecond);

void BmCholeskyQuantLib(benchmark::State& state) {
  const Matrix a = Covariance(100);
  const ql::Matrix m = ToQuantLib(a);
  for (auto _ : state) benchmark::DoNotOptimize(ql::CholeskyDecomposition(m));
  const ql::Matrix l = ql::CholeskyDecomposition(m);
  state.counters["error"] = CholeskyResidual(
      a, [&](std::size_t i, std::size_t k) { return l[i][k]; });
}
BENCHMARK(BmCholeskyQuantLib)->Unit(benchmark::kMicrosecond);

template <typename Get>
double InverseResidual(const Matrix& a, Get inv) {
  const std::size_t n = a.rows();
  double worst = 0.0;
  for (std::size_t i = 0; i < n; ++i) {
    for (std::size_t j = 0; j < n; ++j) {
      double s = 0.0;
      for (std::size_t k = 0; k < n; ++k) s += a(i, k) * inv(k, j);
      worst = std::max(worst, std::fabs(s - (i == j ? 1.0 : 0.0)));
    }
  }
  return worst;
}

void BmInverseInvestm(benchmark::State& state) {
  const Matrix a = Covariance(100);
  for (auto _ : state) benchmark::DoNotOptimize(Inverse(a));
  const Matrix inv = *Inverse(a);
  state.counters["error"] = InverseResidual(
      a, [&](std::size_t i, std::size_t j) { return inv(i, j); });
}
BENCHMARK(BmInverseInvestm)->Unit(benchmark::kMicrosecond);

void BmInverseQuantLib(benchmark::State& state) {
  const Matrix a = Covariance(100);
  const ql::Matrix m = ToQuantLib(a);
  for (auto _ : state) benchmark::DoNotOptimize(ql::inverse(m));
  const ql::Matrix inv = ql::inverse(m);
  state.counters["error"] = InverseResidual(
      a, [&](std::size_t i, std::size_t j) { return inv[i][j]; });
}
BENCHMARK(BmInverseQuantLib)->Unit(benchmark::kMicrosecond);

}  // namespace
}  // namespace investm::compare
