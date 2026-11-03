#include "cpp/ch06_portfolio_selection/utility.h"

#include <cmath>
#include <vector>

#include "cpp/core/check.h"
#include "cpp/core/stats.h"

namespace investm::ch06 {

Utility::Utility(UtilityKind kind, double parameter)
    : kind_(kind), parameter_(parameter) {
  switch (kind_) {
    case UtilityKind::kPower:
      INVESTM_CHECK(parameter_ > 0.0 && parameter_ != 1.0,
                    "power utility needs g > 0, g != 1 (use kLog for 1)");
      break;
    case UtilityKind::kExponential:
    case UtilityKind::kQuadratic:
      INVESTM_CHECK(parameter_ > 0.0, "parameter must be positive");
      break;
    default:
      break;
  }
}

double Utility::operator()(double w) const {
  switch (kind_) {
    case UtilityKind::kLog:
      INVESTM_CHECK(w > 0.0, "log utility needs positive wealth");
      return std::log(w);
    case UtilityKind::kPower:
      INVESTM_CHECK(w > 0.0, "power utility needs positive wealth");
      return std::pow(w, 1.0 - parameter_) / (1.0 - parameter_);
    case UtilityKind::kExponential:
      return -std::exp(-parameter_ * w);
    case UtilityKind::kQuadratic:
      INVESTM_CHECK(w < 0.5 / parameter_, "beyond the bliss point");
      return w - parameter_ * w * w;
    case UtilityKind::kLinear:
      return w;
    case UtilityKind::kConvex:
      return w * w;
  }
  return 0.0;
}

double Utility::Marginal(double w) const {
  switch (kind_) {
    case UtilityKind::kLog:
      return 1.0 / w;
    case UtilityKind::kPower:
      return std::pow(w, -parameter_);
    case UtilityKind::kExponential:
      return parameter_ * std::exp(-parameter_ * w);
    case UtilityKind::kQuadratic:
      return 1.0 - 2.0 * parameter_ * w;
    case UtilityKind::kLinear:
      return 1.0;
    case UtilityKind::kConvex:
      return 2.0 * w;
  }
  return 0.0;
}

double Utility::Inverse(double u) const {
  switch (kind_) {
    case UtilityKind::kLog:
      return std::exp(u);
    case UtilityKind::kPower:
      return std::pow(u * (1.0 - parameter_), 1.0 / (1.0 - parameter_));
    case UtilityKind::kExponential:
      INVESTM_CHECK(u < 0.0, "exponential utility is always negative");
      return -std::log(-u) / parameter_;
    case UtilityKind::kQuadratic:
      // Smaller root of b W^2 - W + u = 0 (the increasing branch).
      return (1.0 - std::sqrt(1.0 - 4.0 * parameter_ * u)) / (2.0 * parameter_);
    case UtilityKind::kLinear:
      return u;
    case UtilityKind::kConvex:
      return std::sqrt(u);
  }
  return 0.0;
}

double Utility::AbsoluteRiskAversion(double w) const {
  switch (kind_) {
    case UtilityKind::kLog:
      return 1.0 / w;
    case UtilityKind::kPower:
      return parameter_ / w;
    case UtilityKind::kExponential:
      return parameter_;
    case UtilityKind::kQuadratic:
      return 2.0 * parameter_ / (1.0 - 2.0 * parameter_ * w);
    case UtilityKind::kLinear:
      return 0.0;
    case UtilityKind::kConvex:
      return -1.0 / w;
  }
  return 0.0;
}

double ExpectedUtility(const Utility& u, std::span<const double> wealth,
                       std::span<const double> probs) {
  INVESTM_CHECK(wealth.size() == probs.size(), "size mismatch");
  double eu = 0.0;
  for (std::size_t i = 0; i < wealth.size(); ++i) eu += probs[i] * u(wealth[i]);
  return eu;
}

double CertaintyEquivalent(const Utility& u, std::span<const double> wealth,
                           std::span<const double> probs) {
  return u.Inverse(ExpectedUtility(u, wealth, probs));
}

double RiskPremium(const Utility& u, std::span<const double> wealth,
                   std::span<const double> probs) {
  return stats::Expected(wealth, probs) - CertaintyEquivalent(u, wealth, probs);
}

double MeanVarianceUtility(double expected_return, double std_dev, double tau) {
  INVESTM_CHECK(tau > 0.0, "risk tolerance must be positive");
  return expected_return - std_dev * std_dev / tau;
}

std::vector<double> IndifferenceCurve(double utility_level,
                                      std::span<const double> sigmas,
                                      double tau) {
  INVESTM_CHECK(tau > 0.0, "risk tolerance must be positive");
  std::vector<double> e;
  e.reserve(sigmas.size());
  for (double s : sigmas) e.push_back(utility_level + s * s / tau);
  return e;
}

}  // namespace investm::ch06
