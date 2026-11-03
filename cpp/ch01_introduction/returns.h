// Chapter 1, Introduction.
// Section 1.1.2: risk, return, and diversification.
//
// Two numbers drive the rest of the book: how much a holding earned (return)
// and how uncertain that was (risk, measured by standard deviation).
// EqualWeightPortfolioStdDev shows why diversification works, before
// Chapter 7 proves it.

#ifndef CPP_CH01_INTRODUCTION_RETURNS_H_
#define CPP_CH01_INTRODUCTION_RETURNS_H_

#include <span>
#include <vector>

namespace investm::ch01 {

// (end value + income received - begin value) / begin value.
double HoldingPeriodReturn(double begin_value, double end_value,
                           double income = 0.0);

// Period-by-period simple returns from a price series. `income`, if not
// empty, holds the cash paid at the end of each period (prices.size() - 1
// entries).
std::vector<double> SimpleReturns(std::span<const double> prices,
                                  std::span<const double> income = {});

// Links periodic returns into one cumulative return.
double CumulativeReturn(std::span<const double> returns);

// Compound annual rate equivalent to `cumulative` earned over `years`.
double Annualize(double cumulative, double years);

// Standard deviation of an equally weighted portfolio of `n_assets` assets
// that share volatility `sigma` and pairwise correlation `rho`:
//   var_p = sigma^2 / N + (1 - 1/N) * rho * sigma^2.
// The first term is unique risk and vanishes as N grows. The second is common
// risk and converges to rho * sigma^2, which diversification cannot remove.
double EqualWeightPortfolioStdDev(int n_assets, double sigma, double rho);

}  // namespace investm::ch01

#endif  // CPP_CH01_INTRODUCTION_RETURNS_H_
