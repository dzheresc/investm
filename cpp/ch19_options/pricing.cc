#include "cpp/ch19_options/pricing.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "cpp/core/check.h"
#include "cpp/core/root_find.h"
#include "cpp/core/stats.h"

namespace investm::ch19 {
namespace {

void CheckSpec(const OptionSpec& s) {
  INVESTM_CHECK(s.spot > 0.0 && s.strike > 0.0, "prices must be positive");
  INVESTM_CHECK(s.volatility > 0.0, "volatility must be positive");
  INVESTM_CHECK(s.years > 0.0, "time to expiry must be positive");
}

}  // namespace

Replication OnePeriodBinomial(OptionType type, double spot, double strike,
                              double up, double down, double rate) {
  INVESTM_CHECK(down < 1.0 + rate && 1.0 + rate < up,
                "need down < 1 + r < up for no arbitrage");
  const double cu = Payoff(type, strike, spot * up);
  const double cd = Payoff(type, strike, spot * down);
  const double shares = (cu - cd) / (spot * (up - down));
  const double bond = (cu - shares * spot * up) / (1.0 + rate);
  return {shares, bond, shares * spot + bond};
}

double BinomialPrice(const OptionSpec& spec, int steps, bool american) {
  CheckSpec(spec);
  INVESTM_CHECK(steps > 0, "need at least one step");
  const double dt = spec.years / steps;
  const double u = std::exp(spec.volatility * std::sqrt(dt));
  const double d = 1.0 / u;
  const double growth = std::exp((spec.rate - spec.dividend_yield) * dt);
  const double p = (growth - d) / (u - d);
  INVESTM_CHECK(p > 0.0 && p < 1.0, "too few steps for these parameters");
  const double disc = std::exp(-spec.rate * dt);
  const auto n = static_cast<std::size_t>(steps);
  // With d = 1/u the price at node (i, j) is S u^(2j - i), so stepping back
  // one level multiplies every price by u. Only the O(n) terminal prices
  // use pow; the O(n^2) backward pass for early exercise is multiplications.
  std::vector<double> s(n + 1);
  std::vector<double> v(n + 1);
  const double log_u = std::log(u);
  for (std::size_t j = 0; j <= n; ++j) {
    const double up_moves =
        2.0 * static_cast<double>(j) - static_cast<double>(n);
    s[j] = spec.spot * std::exp(up_moves * log_u);
    v[j] = Payoff(spec.type, spec.strike, s[j]);
  }
  const double pu = disc * p;
  const double pd = disc * (1.0 - p);
  for (std::size_t i = n; i-- > 0;) {
    if (american) {
      for (std::size_t j = 0; j <= i; ++j) {
        s[j] *= u;
        v[j] = std::max(pu * v[j + 1] + pd * v[j],
                        Payoff(spec.type, spec.strike, s[j]));
      }
    } else {
      for (std::size_t j = 0; j <= i; ++j) v[j] = pu * v[j + 1] + pd * v[j];
    }
  }
  return v[0];
}

Greeks BlackScholesGreeks(const OptionSpec& spec) {
  CheckSpec(spec);
  const double s = spec.spot;
  const double k = spec.strike;
  const double t = spec.years;
  const double sig = spec.volatility;
  const double sqrt_t = std::sqrt(t);
  const double d1 = (std::log(s / k) +
                     (spec.rate - spec.dividend_yield + 0.5 * sig * sig) * t) /
                    (sig * sqrt_t);
  const double d2 = d1 - sig * sqrt_t;
  const double dq = std::exp(-spec.dividend_yield * t);
  const double dr = std::exp(-spec.rate * t);
  const double pdf = stats::NormPdf(d1);
  Greeks g{};
  g.gamma = dq * pdf / (s * sig * sqrt_t);
  g.vega = s * dq * pdf * sqrt_t;
  const double decay = -s * dq * pdf * sig / (2.0 * sqrt_t);
  if (spec.type == OptionType::kCall) {
    const double nd1 = stats::NormCdf(d1);
    const double nd2 = stats::NormCdf(d2);
    g.price = s * dq * nd1 - k * dr * nd2;
    g.delta = dq * nd1;
    g.theta =
        decay - spec.rate * k * dr * nd2 + spec.dividend_yield * s * dq * nd1;
    g.rho = k * t * dr * nd2;
  } else {
    const double nd1 = stats::NormCdf(-d1);
    const double nd2 = stats::NormCdf(-d2);
    g.price = k * dr * nd2 - s * dq * nd1;
    g.delta = -dq * nd1;
    g.theta =
        decay + spec.rate * k * dr * nd2 - spec.dividend_yield * s * dq * nd1;
    g.rho = -k * t * dr * nd2;
  }
  return g;
}

double BlackScholesPrice(const OptionSpec& spec) {
  return BlackScholesGreeks(spec).price;
}

std::optional<double> ImpliedVolatility(OptionSpec spec, double price) {
  auto f = [&](double vol) {
    spec.volatility = vol;
    return BlackScholesPrice(spec) - price;
  };
  spec.volatility = 1e-6;
  const double lower = BlackScholesPrice(spec);
  const double upper =
      spec.type == OptionType::kCall
          ? spec.spot * std::exp(-spec.dividend_yield * spec.years)
          : spec.strike * std::exp(-spec.rate * spec.years);
  if (price <= lower || price >= upper) return std::nullopt;
  return BrentExpandUp(f, 1e-6, 1.0);
}

double HistoricalVolatility(std::span<const double> prices,
                            double periods_per_year) {
  INVESTM_CHECK(prices.size() >= 3, "need at least three prices");
  INVESTM_CHECK(periods_per_year > 0.0, "periods per year must be positive");
  std::vector<double> log_rel;
  log_rel.reserve(prices.size() - 1);
  for (std::size_t t = 1; t < prices.size(); ++t) {
    INVESTM_CHECK(prices[t - 1] > 0.0 && prices[t] > 0.0,
                  "prices must be positive");
    log_rel.push_back(std::log(prices[t] / prices[t - 1]));
  }
  return stats::StdDev(log_rel) * std::sqrt(periods_per_year);
}

double WarrantValue(const OptionSpec& spec, double shares, double warrants) {
  INVESTM_CHECK(shares > 0.0 && warrants >= 0.0, "invalid share counts");
  INVESTM_CHECK(spec.type == OptionType::kCall, "warrants are calls");
  return shares / (shares + warrants) * BlackScholesPrice(spec);
}

}  // namespace investm::ch19
