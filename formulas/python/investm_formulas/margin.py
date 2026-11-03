"""Rate of return and margin accounts (equations 1 to 5)."""

__all__ = [
    "rate_of_return",
    "actual_margin_purchase",
    "margin_call_price_purchase",
    "actual_margin_short_sale",
    "margin_call_price_short_sale",
]


def rate_of_return(beginning_wealth: float, ending_wealth: float) -> float:
    """Eq. 1: (end wealth - beginning wealth) / beginning wealth."""
    return (ending_wealth - beginning_wealth) / beginning_wealth


def actual_margin_purchase(shares: float, market_price: float,
                           initial_margin: float,
                           purchase_price: float) -> float:
    """Eq. 2: actual margin in a stock bought on margin."""
    loan = (1.0 - initial_margin) * purchase_price * shares
    value = shares * market_price
    return (value - loan) / value


def margin_call_price_purchase(initial_margin: float, purchase_price: float,
                               maintenance_margin: float) -> float:
    """Eq. 3: market price at which a margin purchaser gets a margin call."""
    return (1.0 - initial_margin) * purchase_price / (1.0 - maintenance_margin)


def actual_margin_short_sale(shares: float, sale_price: float,
                             initial_margin: float,
                             market_price: float) -> float:
    """Eq. 4: actual margin in a short sale."""
    assets = sale_price * shares * (1.0 + initial_margin)
    liability = market_price * shares
    return (assets - liability) / liability


def margin_call_price_short_sale(sale_price: float, initial_margin: float,
                                 maintenance_margin: float) -> float:
    """Eq. 5: market price at which a short seller gets a margin call."""
    return sale_price * (1.0 + initial_margin) / (1.0 + maintenance_margin)
