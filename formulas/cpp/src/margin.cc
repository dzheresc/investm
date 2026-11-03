#include "investm/formulas/margin.h"

namespace investm::formulas {

double RateOfReturn(double beginning_wealth, double ending_wealth) {
  return (ending_wealth - beginning_wealth) / beginning_wealth;
}

double ActualMarginPurchase(double shares, double market_price,
                            double initial_margin, double purchase_price) {
  const double loan = (1.0 - initial_margin) * purchase_price * shares;
  const double value = shares * market_price;
  return (value - loan) / value;
}

double MarginCallPricePurchase(double initial_margin, double purchase_price,
                               double maintenance_margin) {
  return (1.0 - initial_margin) * purchase_price / (1.0 - maintenance_margin);
}

double ActualMarginShortSale(double shares, double sale_price,
                             double initial_margin, double market_price) {
  const double assets = sale_price * shares * (1.0 + initial_margin);
  const double liability = market_price * shares;
  return (assets - liability) / liability;
}

double MarginCallPriceShortSale(double sale_price, double initial_margin,
                                double maintenance_margin) {
  return sale_price * (1.0 + initial_margin) / (1.0 + maintenance_margin);
}

}  // namespace investm::formulas
