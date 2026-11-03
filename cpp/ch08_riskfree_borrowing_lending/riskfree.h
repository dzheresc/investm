// Chapter 8, Riskfree Borrowing and Lending.
// Sections 8.2 (riskfree lending), 8.3 (riskfree borrowing), 8.4 (both),
// Appendix A (different borrowing and lending rates) and Appendix B
// (composition of the investor's optimal portfolio O*).
//
// Adding a riskfree asset turns the curved efficient set into a straight line
// from the riskfree rate through the tangency portfolio T. Every investor
// holds T, levered up or down according to risk tolerance.

#ifndef CPP_CH08_RISKFREE_BORROWING_LENDING_RISKFREE_H_
#define CPP_CH08_RISKFREE_BORROWING_LENDING_RISKFREE_H_

#include <optional>
#include <span>

#include "cpp/ch07_portfolio_analysis/efficient_set.h"
#include "cpp/core/matrix.h"

namespace investm::ch08 {

// 8.2.1 / 8.3.1 Put fraction `x_risky` in a risky portfolio and the rest in
// the riskfree asset; x_risky > 1 means borrowing at `rf`.
struct Combination {
  double expected_return;
  double std_dev;
};
Combination CombineWithRiskfree(double rf, double risky_expected,
                                double risky_std_dev, double x_risky);

// Slope of the line from rf through a risky portfolio: (E - rf) / sigma.
double SharpeRatio(double expected_return, double std_dev, double rf);

// Tangency portfolio with short sales allowed: w proportional to
// S^-1 (mu - rf 1). Returns nullopt if S is singular or rf is at or above the
// minimum-variance portfolio's return (no tangency on the upper branch).
std::optional<ch07::PortfolioPoint> TangencyPortfolio(
    std::span<const double> mu, const Matrix& cov, double rf);

// Tangency portfolio with no short sales: maximizes the Sharpe ratio along
// the no-short-sales efficient set (golden-section search on the risk
// tolerance that indexes the set).
ch07::PortfolioPoint TangencyPortfolioNoShortSales(std::span<const double> mu,
                                                   const Matrix& cov,
                                                   double rf);

// 8.4.2 / Appendix B Fraction of wealth placed in T by an investor who
// maximizes E - sigma^2 / tau:  x = tau (E_T - rf) / (2 sigma_T^2).
double OptimalFractionInTangency(const ch07::PortfolioPoint& tangency,
                                 double rf, double tau);

// Appendix A: lending at r_lend < borrowing at r_borrow (short sales
// allowed). The efficient set is a lending line to T_L, the curved frontier
// between T_L and T_B, then a borrowing line beyond T_B. If no tangency
// exists at r_borrow, the set simply ends with the risky frontier.
enum class Region { kLend, kRiskyOnly, kBorrow };
struct OptimalChoice {
  Region region;
  double fraction_risky;  // Of wealth, in the risky portfolio used.
  double expected_return;
  double std_dev;
  Vector risky_weights;  // Composition of the risky portfolio used.
};
std::optional<OptimalChoice> OptimalWithDifferentRates(
    std::span<const double> mu, const Matrix& cov, double r_lend,
    double r_borrow, double tau);

}  // namespace investm::ch08

#endif  // CPP_CH08_RISKFREE_BORROWING_LENDING_RISKFREE_H_
