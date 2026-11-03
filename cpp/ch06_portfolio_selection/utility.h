// Chapter 6, The Portfolio Selection Problem.
// Sections 6.2 (nonsatiation and risk aversion), 6.3 (utility, marginal
// utility, certainty equivalents and risk premiums), 6.4 (indifference
// curves) and Appendix B (risk-neutral and risk-seeking investors).

#ifndef CPP_CH06_PORTFOLIO_SELECTION_UTILITY_H_
#define CPP_CH06_PORTFOLIO_SELECTION_UTILITY_H_

#include <span>
#include <vector>

namespace investm::ch06 {

// Utility of wealth. Every kind is increasing in wealth (nonsatiation); the
// curvature decides the attitude towards risk.
enum class UtilityKind {
  kLog,          // ln(W): risk averse, decreasing absolute risk aversion.
  kPower,        // W^(1-g)/(1-g), g = relative risk aversion (g != 1).
  kExponential,  // -exp(-a W), a = absolute risk aversion.
  kQuadratic,    // W - b W^2, valid for W < 1/(2b).
  kLinear,       // W: risk neutral (Appendix B).
  kConvex,       // W^2: risk seeking (Appendix B), W > 0.
};

class Utility {
 public:
  explicit Utility(UtilityKind kind, double parameter = 0.0);

  double operator()(double wealth) const;
  // 6.3.1 Marginal utility dU/dW.
  double Marginal(double wealth) const;
  // Inverse of the utility function: the wealth that delivers utility u.
  double Inverse(double u) const;
  // -U''/U' at the given wealth (Arrow-Pratt absolute risk aversion).
  double AbsoluteRiskAversion(double wealth) const;

 private:
  UtilityKind kind_;
  double parameter_;
};

// Expected utility of a discrete gamble over terminal wealth.
double ExpectedUtility(const Utility& u, std::span<const double> wealth,
                       std::span<const double> probs);

// 6.3.2 The sure wealth the investor values as much as the gamble.
double CertaintyEquivalent(const Utility& u, std::span<const double> wealth,
                           std::span<const double> probs);

// 6.3.2 Expected wealth minus the certainty equivalent. Positive for a
// risk-averse investor, zero if risk neutral, negative if risk seeking.
double RiskPremium(const Utility& u, std::span<const double> wealth,
                   std::span<const double> probs);

// 6.4 Mean-variance indifference curves. An investor with risk tolerance
// `tau` ranks portfolios by  U = E - sigma^2 / tau;  every portfolio on one
// curve has the same U. Returns the expected return needed at each sigma.
double MeanVarianceUtility(double expected_return, double std_dev, double tau);
std::vector<double> IndifferenceCurve(double utility_level,
                                      std::span<const double> sigmas,
                                      double tau);

}  // namespace investm::ch06

#endif  // CPP_CH06_PORTFOLIO_SELECTION_UTILITY_H_
