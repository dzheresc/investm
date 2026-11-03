// Rate of return and margin accounts: equations 1 to 5 of the Investment
// Management Equations sheet. Rates and margins are decimals.

#ifndef INVESTM_FORMULAS_MARGIN_H_
#define INVESTM_FORMULAS_MARGIN_H_

namespace investm::formulas {

// Eq. 1: (end wealth - beginning wealth) / beginning wealth.
double RateOfReturn(double beginning_wealth, double ending_wealth);

// Eq. 2: actual margin in a stock bought on margin.
double ActualMarginPurchase(double shares, double market_price,
                            double initial_margin, double purchase_price);

// Eq. 3: market price at which a margin purchaser receives a margin call.
double MarginCallPricePurchase(double initial_margin, double purchase_price,
                               double maintenance_margin);

// Eq. 4: actual margin in a short sale.
double ActualMarginShortSale(double shares, double sale_price,
                             double initial_margin, double market_price);

// Eq. 5: market price at which a short seller receives a margin call.
double MarginCallPriceShortSale(double sale_price, double initial_margin,
                                double maintenance_margin);

}  // namespace investm::formulas

#endif  // INVESTM_FORMULAS_MARGIN_H_
