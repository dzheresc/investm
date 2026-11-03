// Chapter 17, The Valuation of Common Stocks.
// Dividend discount models (zero growth, constant growth, multiple growth
// and three-phase), implied returns, and the links between dividend
// discount models, price-earnings ratios and growth opportunities.

#ifndef CPP_CH17_DDM_DDM_H_
#define CPP_CH17_DDM_DDM_H_

#include <optional>
#include <span>
#include <vector>

namespace investm::ch17 {

// 17.2 Present value of dividends D_1..D_n paid at the end of years 1..n.
double PresentValueOfDividends(std::span<const double> dividends, double k);

// 17.3 Zero growth: V = D / k.
double ZeroGrowthValue(double dividend, double k);
// 17.4 Constant growth (Gordon): V = D_0 (1 + g) / (k - g), k > g.
double ConstantGrowthValue(double d0, double g, double k);
// Implied return of the constant-growth model: D_1 / P + g.
double ConstantGrowthImpliedReturn(double price, double d0, double g);

// 17.5 Multiple growth: explicit dividends D_1..D_T, then growth g forever.
//   V = sum D_t / (1+k)^t + D_T (1 + g) / ((k - g)(1 + k)^T).
double MultipleGrowthValue(std::span<const double> explicit_dividends, double g,
                           double k);
// k that equates the multiple-growth value with `price`; nullopt if none
// above g exists.
std::optional<double> MultipleGrowthImpliedReturn(
    double price, std::span<const double> explicit_dividends, double g);

// 17.6 Three-phase model: dividends grow at g_a for years 1..t1, the growth
// rate then falls linearly to g_c by year t2, and stays at g_c forever.
// Returns D_1..D_t2.
std::vector<double> ThreePhaseDividends(double d0, double g_a, int t1, int t2,
                                        double g_c);
double ThreePhaseValue(double d0, double g_a, int t1, int t2, double g_c,
                       double k);

// Net present value of buying at `price` when the model says `value`.
inline double NetPresentValue(double value, double price) {
  return value - price;
}

// 17.7 Models based on price-earnings ratios. With payout ratio p and
// constant growth: forward P/E = p / (k - g); trailing P/E multiplies by
// (1 + g).
double ForwardPriceEarnings(double payout, double g, double k);
double TrailingPriceEarnings(double payout, double g, double k);

// Sustainable growth from reinvestment: g = ROE x retention.
double SustainableGrowth(double roe, double retention);
// Value when a firm retains `retention` of earnings at return `roe`:
// E_1 (1 - b) / (k - b ROE).
double ValueWithReinvestment(double e1, double retention, double roe, double k);
// Present value of growth opportunities: P - E_1 / k.
double PresentValueOfGrowthOpportunities(double price, double e1, double k);

}  // namespace investm::ch17

#endif  // CPP_CH17_DDM_DDM_H_
