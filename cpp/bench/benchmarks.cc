// Benchmarks of the computational kernels behind the chapter libraries.
//
//   build/cpp/bench/investm_bench                  # timing table
//   build/cpp/bench/investm_bench --filter=eigen   # a subset
//   build/cpp/bench/investm_bench --quick          # one call each (valgrind)
//
// Inputs are generated from a fixed seed, so every run does the same work.

#include <algorithm>
#include <cmath>
#include <random>
#include <string>
#include <vector>

#include "cpp/bench/bench.h"
#include "cpp/ch05_riskless_securities/cash_flows.h"
#include "cpp/ch05_riskless_securities/term_structure.h"
#include "cpp/ch07_portfolio_analysis/efficient_set.h"
#include "cpp/ch08_riskfree_borrowing_lending/riskfree.h"
#include "cpp/ch10_factor_models/factor_models.h"
#include "cpp/ch13_fixed_income/fixed_income.h"
#include "cpp/ch14_bond_analysis/duration.h"
#include "cpp/ch19_options/pricing.h"
#include "cpp/ch23_investment_management/management.h"
#include "cpp/ch24_performance/performance.h"
#include "cpp/core/matrix.h"
#include "cpp/core/regression.h"

namespace {

using investm::Matrix;
using investm::Vector;
using investm::bench::Case;
using investm::bench::DoNotOptimize;
namespace ch05 = investm::ch05;
namespace ch07 = investm::ch07;
namespace ch08 = investm::ch08;
namespace ch10 = investm::ch10;
namespace ch13 = investm::ch13;
namespace ch14 = investm::ch14;
namespace ch19 = investm::ch19;
namespace ch23 = investm::ch23;
namespace ch24 = investm::ch24;

// T x N matrix of returns driven by three factors plus noise.
Matrix FactorReturns(std::size_t t, std::size_t n, unsigned seed) {
  std::mt19937 rng(seed);
  std::normal_distribution<double> z(0.0, 1.0);
  std::vector<Vector> loadings(n, Vector(3));
  for (auto& b : loadings) {
    for (double& x : b) x = 0.5 + 0.5 * z(rng);
  }
  Matrix r(t, n);
  for (std::size_t i = 0; i < t; ++i) {
    const double f[3] = {0.04 * z(rng), 0.02 * z(rng), 0.015 * z(rng)};
    for (std::size_t j = 0; j < n; ++j) {
      r(i, j) = 0.006 + loadings[j][0] * f[0] + loadings[j][1] * f[1] +
                loadings[j][2] * f[2] + 0.05 * z(rng);
    }
  }
  return r;
}

Vector ExpectedReturns(std::size_t n) {
  Vector mu(n);
  for (std::size_t i = 0; i < n; ++i) {
    mu[i] = 0.04 + 0.08 * static_cast<double>(i) / static_cast<double>(n);
  }
  return mu;
}

Matrix RandomMatrix(std::size_t rows, std::size_t cols, unsigned seed) {
  std::mt19937 rng(seed);
  std::uniform_real_distribution<double> u(-1.0, 1.0);
  Matrix m(rows, cols);
  for (std::size_t i = 0; i < rows; ++i) {
    for (std::size_t j = 0; j < cols; ++j) m(i, j) = u(rng);
  }
  return m;
}

double Trace(const Matrix& m) {
  double s = 0.0;
  for (std::size_t i = 0; i < std::min(m.rows(), m.cols()); ++i) s += m(i, i);
  return s;
}

std::vector<Case> MakeCases() {
  std::vector<Case> cases;

  // --- core linear algebra -------------------------------------------------
  for (std::size_t n : {std::size_t{50}, std::size_t{200}}) {
    const Matrix a = RandomMatrix(n, n, 1);
    const Matrix b = RandomMatrix(n, n, 2);
    const Matrix spd = investm::CovarianceMatrix(FactorReturns(2 * n, n, 3));
    const Vector rhs(n, 1.0);
    const std::string s = std::to_string(n);
    cases.push_back({"core/matmul/" + s, [=] { DoNotOptimize(Trace(a * b)); }});
    cases.push_back({"core/solve/" + s, [=] {
                       DoNotOptimize(investm::Solve(a, rhs).value()[0]);
                     }});
    cases.push_back({"core/inverse/" + s, [=] {
                       DoNotOptimize(Trace(investm::Inverse(a).value()));
                     }});
    cases.push_back({"core/cholesky/" + s, [=] {
                       DoNotOptimize(Trace(investm::Cholesky(spd).value()));
                     }});
    cases.push_back(
        {"core/eigen_symmetric/" + s,
         [=] { DoNotOptimize(investm::EigenSymmetric(spd).values.front()); }});
  }
  {
    const Matrix data = FactorReturns(1000, 100, 4);
    cases.push_back({"core/covariance_matrix/1000x100", [=] {
                       DoNotOptimize(Trace(investm::CovarianceMatrix(data)));
                     }});
    const Matrix x = RandomMatrix(2000, 10, 5);
    Vector y(2000);
    for (std::size_t i = 0; i < y.size(); ++i) y[i] = x(i, 0) + 0.1 * x(i, 1);
    cases.push_back(
        {"core/multiple_ols/2000x10",
         [=] { DoNotOptimize(investm::MultipleOls(x, y)->coefficients[1]); }});
  }

  // --- ch05 riskless securities --------------------------------------------
  {
    const auto bond = ch05::CouponBond(100.0, 0.045, 30.0, 2);
    cases.push_back(
        {"ch05/ytm/30y_semiannual",
         [=] { DoNotOptimize(ch05::YieldToMaturity(bond, 93.5, 2).value()); }});
    std::vector<ch05::BondQuote> quotes;
    for (int t = 1; t <= 30; ++t) {
      const double c = 0.03 + 0.0005 * t;
      const double y = 0.032 + 0.0006 * t;
      quotes.push_back(
          {100.0, c,
           ch05::PresentValueAtYield(ch05::CouponBond(100.0, c, t), y)});
    }
    cases.push_back({"ch05/bootstrap/30", [=] {
                       DoNotOptimize(ch05::Bootstrap(quotes)->Spot(30));
                     }});
  }

  // --- ch07/ch08 portfolio optimization ------------------------------------
  for (std::size_t n : {std::size_t{20}, std::size_t{100}}) {
    const Vector mu = ExpectedReturns(n);
    const Matrix cov = investm::CovarianceMatrix(FactorReturns(3 * n, n, 6));
    const std::string s = std::to_string(n);
    cases.push_back({"ch07/frontier_create/" + s, [=] {
                       DoNotOptimize(
                           ch07::UnconstrainedFrontier::Create(mu, cov)->a());
                     }});
    cases.push_back({"ch07/optimal_no_short_sales/" + s, [=] {
                       DoNotOptimize(
                           ch07::OptimalNoShortSales(mu, cov, 0.5).std_dev);
                     }});
    const std::vector<double> taus{0.05, 0.1, 0.2, 0.4, 0.8, 1.6, 3.2, 6.4};
    cases.push_back(
        {"ch07/efficient_set_no_short/" + s + "x8", [=] {
           DoNotOptimize(
               ch07::EfficientSetNoShortSales(mu, cov, taus).back().std_dev);
         }});
    cases.push_back(
        {"ch08/tangency_no_short/" + s, [=] {
           DoNotOptimize(
               ch08::TangencyPortfolioNoShortSales(mu, cov, 0.03).std_dev);
         }});
  }

  // --- ch10 factor models --------------------------------------------------
  {
    const Matrix r = FactorReturns(250, 100, 7);
    cases.push_back(
        {"ch10/principal_components/250x100", [=] {
           DoNotOptimize(
               ch10::EstimatePrincipalComponents(r, 3).explained_fraction);
         }});
  }

  // --- ch13/ch14 bonds -----------------------------------------------------
  {
    using std::chrono::day;
    using std::chrono::month;
    using std::chrono::year;
    const ch13::Bond note{100.0, 0.0425,
                          ch13::Date{year{2054}, month{2}, day{15}}, 2,
                          ch13::DayCount::kActualActual};
    const ch13::Date settle{year{2026}, month{11}, day{3}};
    cases.push_back(
        {"ch13/yield_from_clean_price/28y", [=] {
           DoNotOptimize(ch13::YieldFromCleanPrice(note, settle, 96.4).value());
         }});
    const auto flows = ch05::CouponBond(100.0, 0.05, 30.0, 2);
    cases.push_back({"ch14/duration_convexity/30y", [=] {
                       DoNotOptimize(ch14::ModifiedDuration(flows, 0.05, 2) +
                                     ch14::Convexity(flows, 0.05, 2));
                     }});
  }

  // --- ch19 options --------------------------------------------------------
  {
    const ch19::OptionSpec put{
        ch19::OptionType::kPut, 100.0, 105.0, 0.05, 0.3, 1.0, 0.01};
    for (int steps : {100, 1000}) {
      const std::string s = std::to_string(steps);
      cases.push_back({"ch19/binomial_european/" + s, [=] {
                         DoNotOptimize(ch19::BinomialPrice(put, steps, false));
                       }});
      cases.push_back({"ch19/binomial_american/" + s, [=] {
                         DoNotOptimize(ch19::BinomialPrice(put, steps, true));
                       }});
    }
    cases.push_back({"ch19/black_scholes_greeks", [=] {
                       DoNotOptimize(ch19::BlackScholesGreeks(put).gamma);
                     }});
    const double price = ch19::BlackScholesPrice(put);
    cases.push_back(
        {"ch19/implied_volatility",
         [=] { DoNotOptimize(ch19::ImpliedVolatility(put, price).value()); }});
  }

  // --- ch23/ch24 management and evaluation ---------------------------------
  {
    std::mt19937 rng(8);
    std::normal_distribution<double> z(0.007, 0.045);
    Vector path(120);
    for (double& x : path) x = z(rng);
    cases.push_back({"ch23/cppi/120", [=] {
                       DoNotOptimize(
                           ch23::Cppi(path, 0.003, 3.0, 0.8).wealth.back());
                     }});
    cases.push_back(
        {"ch23/constant_mix/120", [=] {
           DoNotOptimize(
               ch23::ConstantMix(path, 0.003, 0.6, 0.05).wealth.back());
         }});
    const Matrix styles = FactorReturns(120, 8, 9);
    Vector fund(120);
    for (std::size_t t = 0; t < fund.size(); ++t) {
      fund[t] = 0.3 * styles(t, 1) + 0.5 * styles(t, 4) + 0.2 * styles(t, 6);
    }
    cases.push_back(
        {"ch24/style_analysis/120x8",
         [=] { DoNotOptimize(ch24::StyleAnalysis(fund, styles).r_squared); }});
  }
  return cases;
}

}  // namespace

int main(int argc, char** argv) {
  return investm::bench::RunAll(MakeCases(),
                                investm::bench::ParseOptions(argc, argv));
}
