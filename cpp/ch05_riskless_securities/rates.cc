#include "cpp/ch05_riskless_securities/rates.h"

#include <cmath>

#include "cpp/core/check.h"

namespace investm::ch05 {

double RealRate(double nominal, double inflation) {
  return (1.0 + nominal) / (1.0 + inflation) - 1.0;
}

double NominalRate(double real, double inflation) {
  return (1.0 + real) * (1.0 + inflation) - 1.0;
}

double EffectiveAnnualRate(double apr, int m) {
  INVESTM_CHECK(m > 0, "compounding frequency must be positive");
  return std::pow(1.0 + apr / m, m) - 1.0;
}

double AprFromEffective(double effective, int m) {
  INVESTM_CHECK(m > 0, "compounding frequency must be positive");
  return m * (std::pow(1.0 + effective, 1.0 / m) - 1.0);
}

double ConvertCompounding(double apr, int m1, int m2) {
  return AprFromEffective(EffectiveAnnualRate(apr, m1), m2);
}

double FutureValue(double present_value, double apr, int m, double years) {
  INVESTM_CHECK(m > 0, "compounding frequency must be positive");
  return present_value * std::pow(1.0 + apr / m, m * years);
}

double EffectiveFromContinuous(double continuous_rate) {
  return std::expm1(continuous_rate);
}

double ContinuousFromEffective(double effective) {
  return std::log1p(effective);
}

double FutureValueContinuous(double present_value, double continuous_rate,
                             double years) {
  return present_value * std::exp(continuous_rate * years);
}

double BillPriceFromDiscount(double face, double discount_yield, int days) {
  return face * (1.0 - discount_yield * days / 360.0);
}

double BankDiscountYield(double face, double price, int days) {
  INVESTM_CHECK(days > 0, "days must be positive");
  return (face - price) / face * 360.0 / days;
}

double BondEquivalentYield(double face, double price, int days) {
  INVESTM_CHECK(days > 0, "days must be positive");
  return (face - price) / price * 365.0 / days;
}

double MoneyMarketYield(double face, double price, int days) {
  INVESTM_CHECK(days > 0, "days must be positive");
  return (face - price) / price * 360.0 / days;
}

double BillEffectiveAnnualYield(double face, double price, int days) {
  INVESTM_CHECK(days > 0, "days must be positive");
  return std::pow(face / price, 365.0 / days) - 1.0;
}

}  // namespace investm::ch05
