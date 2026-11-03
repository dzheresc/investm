// Worked examples for Chapter 11. All numbers are illustrative.

#include <cmath>
#include <cstdio>
#include <vector>

#include "cpp/ch11_apt/apt.h"
#include "cpp/core/table.h"

namespace {

using investm::Matrix;
using investm::PrintHeading;
using investm::PrintPercent;
using investm::Vector;
namespace ch11 = investm::ch11;

void ArbitragePortfolio() {
  PrintHeading("11.1 An arbitrage opportunity in a one-factor world");
  const Matrix b({{0.9}, {1.1}, {1.5}, {0.5}});
  const Vector mu{0.10, 0.12, 0.20, 0.07};
  std::printf("  stock  b      E[r]\n");
  for (std::size_t i = 0; i < 4; ++i) {
    std::printf("  %zu      %.2f   %.2f%%\n", i + 1, b(i, 0), 100 * mu[i]);
  }
  const Vector x = ch11::FindArbitragePortfolio(mu, b, 100000.0).value();
  std::printf("  arbitrage portfolio ($100,000 of purchases):\n");
  double cost = 0.0;
  double sens = 0.0;
  double gain = 0.0;
  for (std::size_t i = 0; i < 4; ++i) {
    std::printf("    stock %zu: %+12.2f\n", i + 1, x[i]);
    cost += x[i];
    sens += x[i] * b(i, 0);
    gain += x[i] * mu[i];
  }
  std::printf("  net cost %.2f, factor exposure %.2e, expected profit $%.2f\n",
              std::fabs(cost) < 1e-6 ? 0.0 : cost, sens, gain);
  std::printf(
      "  Buying stock 3 and selling the others bids its price up until its\n"
      "  expected return falls back onto the APT line (11.2).\n");
}

void AptPlane() {
  PrintHeading("11.4 A two-factor APT plane");
  const Matrix b({{1.0, 0.2},
                  {0.5, 1.0},
                  {1.2, -0.3},
                  {0.8, 0.7},
                  {0.3, 0.1},
                  {1.5, 0.4}});
  const double lambda0 = 0.035;
  const Vector lambdas{0.045, 0.015};
  Vector mu;
  for (std::size_t i = 0; i < b.rows(); ++i) {
    mu.push_back(ch11::AptExpectedReturn(lambda0, lambdas, b.Row(i)));
  }
  const auto fit = ch11::FitAptPricing(mu, b).value();
  PrintPercent("fitted lambda_0 (zero-beta rate)", fit.lambda0);
  PrintPercent("fitted lambda_1 (factor 1 premium)", fit.lambdas[0]);
  PrintPercent("fitted lambda_2 (factor 2 premium)", fit.lambdas[1]);
  for (std::size_t k = 0; k < 2; ++k) {
    const Vector w = ch11::PureFactorPortfolio(b, k).value();
    std::printf("  pure factor-%zu portfolio earns %.3f%% =", k + 1,
                100 * investm::Dot(w, mu));
    std::printf(" lambda_0 + lambda_%zu\n", k + 1);
  }
}

void AptVersusCapm() {
  PrintHeading("11.5 Synthesis: one factor, the market portfolio");
  std::printf(
      "  If the only factor is the market return, b_i is beta and\n"
      "  lambda_1 = E[r_M] - rf, so the APT line is the SML:\n");
  const double rf = 0.03;
  const double premium = 0.06;
  for (double beta : {0.5, 1.0, 1.5}) {
    std::printf(
        "    beta %.1f -> %.2f%%\n", beta,
        100 * ch11::AptExpectedReturn(rf, Vector{premium}, Vector{beta}));
  }
}

}  // namespace

int main() {
  ArbitragePortfolio();
  AptPlane();
  AptVersusCapm();
  return 0;
}
