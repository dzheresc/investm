// Chapter 4, Efficient Markets, Investment Value, and Market Price.
// Section 4.2.3: security price changes as a random walk.
// Section 4.3: testing for market efficiency. Event studies (4.3.1) and
// looking for patterns (4.3.2).

#ifndef CPP_CH04_EFFICIENT_MARKETS_EFFICIENCY_TESTS_H_
#define CPP_CH04_EFFICIENT_MARKETS_EFFICIENCY_TESTS_H_

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

namespace investm::ch04 {

// Geometric random walk: n iid normal log returns with mean `mu` and standard
// deviation `sigma`, starting from `p0`. Returns n + 1 prices.
std::vector<double> RandomWalkPrices(double p0, std::size_t n, double mu,
                                     double sigma, std::uint64_t seed);

// Natural-log returns of a price series.
std::vector<double> LogReturns(std::span<const double> prices);

// Wald-Wolfowitz runs test on the signs of returns (zeros dropped). Under
// randomness E[R] = 2 n1 n2 / n + 1. |z| > 1.96 rejects randomness at 5%.
struct RunsTestResult {
  std::size_t runs;
  double expected_runs;
  double z;
};
RunsTestResult RunsTest(std::span<const double> returns);

// Variance ratio VR(q) = Var(q-period return) / (q * Var(1-period return)),
// using overlapping sums of log returns. About 1 under a random walk; above 1
// suggests momentum, below 1 mean reversion.
double VarianceRatio(std::span<const double> log_returns, std::size_t q);

// Filter rule: buy after the price rises `x` from its latest trough, move to
// cash after it falls `x` from its latest peak. Returns the strategy's
// cumulative return, to compare with buy-and-hold.
double FilterRuleReturn(std::span<const double> prices, double x);

// Event study. Each firm's series is an estimation window followed by an event
// window. The market model is fitted on the estimation window; abnormal
// returns AR = r - (alpha + beta * r_m) are averaged across firms (AAR) and
// cumulated (CAAR).
struct EventFirm {
  std::vector<double> stock;
  std::vector<double> market;
};

struct EventStudyResult {
  std::vector<double> aar;
  std::vector<double> caar;
  // t statistic of the final CAAR, using estimation-window residual variance.
  double caar_t_stat = 0.0;
};

EventStudyResult EventStudy(std::span<const EventFirm> firms,
                            std::size_t estimation_length);

}  // namespace investm::ch04

#endif  // CPP_CH04_EFFICIENT_MARKETS_EFFICIENCY_TESTS_H_
