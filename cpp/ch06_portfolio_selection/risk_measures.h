// Chapter 6, The Portfolio Selection Problem.
// Institutional issues: alternative risk measures. Standard deviation treats
// upside and downside surprises alike; these measures focus on the downside.

#ifndef CPP_CH06_PORTFOLIO_SELECTION_RISK_MEASURES_H_
#define CPP_CH06_PORTFOLIO_SELECTION_RISK_MEASURES_H_

#include <span>

namespace investm::ch06 {

// Mean of squared shortfalls below `target` over all observations.
double SemiVariance(std::span<const double> returns, double target);
// sqrt(SemiVariance): the downside deviation.
double DownsideDeviation(std::span<const double> returns, double target);
// Fraction of observations below `target`.
double ShortfallProbability(std::span<const double> returns, double target);

// Historical value at risk at confidence `alpha` (e.g. 0.95): the loss that is
// exceeded in only (1 - alpha) of the observations, reported as a positive
// number.
double HistoricalVaR(std::span<const double> returns, double alpha);
// Average loss in the worst (1 - alpha) tail (expected shortfall / CVaR).
double HistoricalExpectedShortfall(std::span<const double> returns,
                                   double alpha);

// Parametric (normal) VaR and expected shortfall: losses as positive numbers.
double NormalVaR(double mean, double std_dev, double alpha);
double NormalExpectedShortfall(double mean, double std_dev, double alpha);

// Inverse of the standard normal CDF (Acklam's rational approximation
// refined by one Newton step).
double NormInv(double p);

}  // namespace investm::ch06

#endif  // CPP_CH06_PORTFOLIO_SELECTION_RISK_MEASURES_H_
