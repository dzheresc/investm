// Worked examples for Chapter 7. All numbers are illustrative.

#include <cmath>
#include <cstdio>
#include <random>
#include <vector>

#include "cpp/ch07_portfolio_analysis/efficient_set.h"
#include "cpp/ch07_portfolio_analysis/market_model.h"
#include "cpp/core/table.h"

namespace {

using investm::Matrix;
using investm::PrintHeading;
using investm::PrintPercent;
using investm::Vector;
namespace ch07 = investm::ch07;

// Five illustrative asset classes.
const Vector kMu{0.045, 0.065, 0.085, 0.095, 0.110};
const Vector kSd{0.06, 0.10, 0.16, 0.20, 0.26};
Matrix Cov() {
  const Matrix corr({{1.00, 0.40, 0.10, 0.05, 0.00},
                     {0.40, 1.00, 0.35, 0.30, 0.20},
                     {0.10, 0.35, 1.00, 0.75, 0.60},
                     {0.05, 0.30, 0.75, 1.00, 0.65},
                     {0.00, 0.20, 0.60, 0.65, 1.00}});
  return investm::CovarianceFromCorrelation(kSd, corr);
}

void PrintWeights(const char* label, const ch07::PortfolioPoint& p) {
  std::printf("  %-26s E=%6.2f%% sd=%6.2f%%  w:", label,
              100 * p.expected_return, 100 * p.std_dev);
  for (double w : p.weights) std::printf(" %+6.3f", w);
  std::printf("\n");
}

void TwoSecurityBounds() {
  PrintHeading("7.2 Concavity: two securities under different correlations");
  std::printf("  A: E=8%%, sd=15%%   B: E=14%%, sd=30%%\n");
  investm::Table table({"x_A", "E %", "rho=+1", "rho=0", "rho=-1"});
  for (double x = 1.0; x >= -0.001; x -= 0.125) {
    const double e = 100 * (x * 0.08 + (1 - x) * 0.14);
    auto sd = [&](double rho) {
      return 100 * std::sqrt(x * x * 0.0225 + (1 - x) * (1 - x) * 0.09 +
                             2 * x * (1 - x) * rho * 0.15 * 0.30);
    };
    table.AddRow({x, e, sd(1.0), sd(0.0), sd(-1.0)}, 3);
  }
  table.Print();
  std::printf("  min-variance weight in A at rho=0: %.4f\n",
              ch07::TwoAssetMinVarianceWeight(0.15, 0.30, 0.0));
}

void EfficientSets() {
  PrintHeading("7.1 / Appendix A Efficient set, short sales allowed");
  const Matrix cov = Cov();
  const auto frontier = ch07::UnconstrainedFrontier::Create(kMu, cov).value();
  PrintWeights("minimum variance", frontier.MinimumVariance());
  for (double e : {0.07, 0.09, 0.11, 0.13}) {
    char label[32];
    std::snprintf(label, sizeof(label), "target E = %.0f%%", 100 * e);
    PrintWeights(label, frontier.ForTargetReturn(e));
  }
  PrintHeading("7.1 / Appendix A Efficient set, no short sales");
  const std::vector<double> taus{0.0, 0.05, 0.1, 0.2, 0.4, 0.8, 1.6, 5.0};
  for (double tau : taus) {
    char label[32];
    std::snprintf(label, sizeof(label), "tau = %.2f", tau);
    PrintWeights(label, ch07::OptimalNoShortSales(kMu, cov, tau));
  }
  std::printf(
      "  Assets enter and leave as tolerance rises; the top-return asset\n"
      "  alone is the efficient set's upper end point.\n");
  PrintHeading("7.1.3 Selecting the optimal portfolio (tau = 0.4)");
  PrintWeights("with short sales", frontier.Optimal(0.4));
  PrintWeights("without short sales", ch07::OptimalNoShortSales(kMu, cov, 0.4));
}

void MarketModelAndDiversification() {
  PrintHeading("7.3 The market model, estimated on simulated monthly data");
  std::mt19937_64 rng(3);
  std::normal_distribution<double> index_dist(0.008, 0.045);
  std::normal_distribution<double> noise(0.0, 0.06);
  const double true_betas[] = {0.6, 0.9, 1.1, 1.4};
  std::vector<double> index(120);
  for (double& r : index) r = index_dist(rng);
  std::vector<ch07::MarketModel> models;
  investm::Table table({"true beta", "beta", "alpha %", "R^2", "sd(e) %"});
  for (double b : true_betas) {
    std::vector<double> stock;
    for (double ri : index) stock.push_back(0.001 + b * ri + noise(rng));
    const ch07::MarketModel m = ch07::FitMarketModel(stock, index);
    models.push_back(m);
    table.AddRow({b, m.beta, 100 * m.alpha, m.r_squared,
                  100 * std::sqrt(m.residual_variance)},
                 3);
  }
  table.Print();

  PrintHeading("7.4 Portfolio market and unique risk");
  const double index_var = 0.045 * 0.045;
  const std::vector<double> w(4, 0.25);
  const auto risk = ch07::PortfolioRiskFromMarketModels(w, models, index_var);
  std::printf("  equal-weight portfolio: beta %.3f\n", risk.beta);
  PrintPercent("market risk (sd)", std::sqrt(risk.market_variance));
  PrintPercent("unique risk (sd)", std::sqrt(risk.unique_variance));
  PrintPercent("total risk (sd)", std::sqrt(risk.total_variance));

  PrintHeading("7.4.5 Random diversification");
  std::printf("  average variance 0.16, average covariance 0.03\n");
  for (int n : {1, 2, 5, 10, 20, 50, 100, 500}) {
    std::printf(
        "  N = %3d  sigma_P = %6.2f%%\n", n,
        100 * std::sqrt(ch07::RandomDiversificationVariance(n, 0.16, 0.03)));
  }
}

}  // namespace

int main() {
  TwoSecurityBounds();
  EfficientSets();
  MarketModelAndDiversification();
  return 0;
}
