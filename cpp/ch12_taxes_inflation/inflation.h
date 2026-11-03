// Chapter 12, Taxes and Inflation.
// Section 12.2: inflation. Rates of inflation from a price index, real
// returns, the Fisher relation, the combined bite of taxes and inflation,
// inflation-indexed bonds and the inflation sensitivity of an asset.

#ifndef CPP_CH12_TAXES_INFLATION_INFLATION_H_
#define CPP_CH12_TAXES_INFLATION_INFLATION_H_

#include <span>
#include <vector>

#include "cpp/ch05_riskless_securities/cash_flows.h"
#include "cpp/core/stats.h"

namespace investm::ch12 {

// 12.2.1 Period inflation rates I_t / I_{t-1} - 1 from index levels.
std::vector<double> InflationRates(std::span<const double> index_levels);
// Compound inflation over the whole series: I_T / I_0 - 1.
double CumulativeInflation(std::span<const double> index_levels);
// Purchasing power of one unit of currency at the end of the series,
// in units of the base period: I_0 / I_T.
double PurchasingPower(std::span<const double> index_levels);

// 12.2.2 Exact real returns (1 + R) / (1 + I) - 1, element by element.
std::vector<double> RealReturns(std::span<const double> nominal,
                                std::span<const double> inflation);

// 12.2.3 Fisher relation: nominal rate that delivers `real` given expected
// inflation, (1 + r)(1 + i) - 1. See also ch05::NominalRate.
double FisherNominalRate(double real, double expected_inflation);

// 12.2.4 Real after-tax return when the whole nominal return is taxed:
// (1 + R (1 - t)) / (1 + I) - 1.
double RealAfterTaxReturn(double nominal, double tax_rate, double inflation);
// Effective tax rate on the real return: 1 - real_after_tax / real_pretax.
// Exceeds the statutory rate whenever inflation is positive.
double EffectiveTaxOnRealReturn(double nominal, double tax_rate,
                                double inflation);

// 12.2.5 Inflation-indexed (TIPS-style) bond. Principal is scaled by the
// index ratio I_t / I_0; each coupon pays real_coupon/frequency times the
// indexed principal. `index_path[k]` is the index level at coupon date k+1;
// the bond matures at the last date. Principal never falls below face
// at maturity (deflation floor).
ch05::CashFlows IndexedBondCashFlows(double face, double real_coupon,
                                     double base_index,
                                     std::span<const double> index_path,
                                     int frequency = 1);

// Break-even inflation: the inflation rate at which a nominal and an
// indexed bond with yields `nominal_yield` and `real_yield` pay the same.
double BreakevenInflation(double nominal_yield, double real_yield);

// 12.2.6 Inflation beta: OLS of an asset's nominal returns on inflation.
// beta = 1 means a perfect nominal hedge.
stats::SimpleRegression InflationBeta(std::span<const double> nominal_returns,
                                      std::span<const double> inflation);

}  // namespace investm::ch12

#endif  // CPP_CH12_TAXES_INFLATION_INFLATION_H_
