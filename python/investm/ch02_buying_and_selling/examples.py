"""Worked examples for Chapter 2.

Run: python -m investm.ch02_buying_and_selling.examples
"""

from investm.ch02_buying_and_selling import margin
from investm.ch02_buying_and_selling.orders import Order
from investm.ch02_buying_and_selling.orders import OrderType
from investm.ch02_buying_and_selling.orders import Side
from investm.ch02_buying_and_selling.orders import simulate
from investm.core import table


def _show(name: str, order: Order, path: list[float]) -> None:
    fill = simulate(order, path)
    if fill is None:
        print(f"  {name:<36} not filled")
    else:
        print(f"  {name:<36} filled at {fill.price:.2f} (tick {fill.index})")


def main() -> None:
    table.heading("2.3 Order types on one price path")
    path = [50.0, 49.2, 48.5, 47.8, 49.0, 51.5, 52.4, 50.9]
    print("  path: " + " ".join(f"{p:.2f}" for p in path))
    _show("market buy", Order(), path)
    _show("limit buy @ 48.00", Order(Side.BUY, OrderType.LIMIT, 48.0), path)
    _show("limit buy @ 47.00", Order(Side.BUY, OrderType.LIMIT, 47.0), path)
    _show("stop-loss sell @ 48.00", Order(Side.SELL, OrderType.STOP, stop=48.0),
          path)
    _show("stop buy @ 51.00", Order(Side.BUY, OrderType.STOP, stop=51.0), path)
    _show("stop-limit buy, stop 51 / limit 51.5",
          Order(Side.BUY, OrderType.STOP_LIMIT, 51.5, 51.0), path)

    table.heading("2.4.1 Margin purchase: 200 shares at $60, 55% initial")
    mp = margin.MarginPurchase(200, 60.0, 0.55)
    table.value("cost", mp.cost, "$")
    table.value("own money", mp.equity_invested, "$")
    table.value("loan", mp.loan, "$")
    table.percent("actual margin if price falls to $45", mp.actual_margin(45))
    table.value("margin call price (30% maintenance)",
                mp.margin_call_price(0.30), "$")
    table.value("cash to restore 55% margin at $40",
                mp.cash_to_restore(40.0, 0.55), "$")
    table.percent("return if sold at $72, 9% loan rate",
                  mp.return_on_equity(72.0, 0.0, 0.09))
    table.percent("same trade without margin", (72.0 - 60.0) / 60.0)

    table.heading("2.4.2 Short sale: 300 shares at $80, 50% initial")
    ss = margin.ShortSale(300, 80.0, 0.50)
    table.value("proceeds held by broker", ss.proceeds, "$")
    table.value("investor deposit", ss.deposit, "$")
    table.percent("actual margin if price rises to $95", ss.actual_margin(95))
    table.value("margin call price (30% maintenance)",
                ss.margin_call_price(0.30), "$")
    table.percent("return if covered at $64, $1 dividend",
                  ss.return_on_equity(64.0, 1.0))

    table.heading("2.4.4 Aggregation: one long and one short position")
    account = margin.Account(
        [margin.Position(200, 60.0),
         margin.Position(-300, 80.0)], -mp.loan + ss.proceeds + ss.deposit)
    table.value("equity", account.equity, "$")
    table.percent("account margin", account.actual_margin())
    account.positions = [margin.Position(200, 50.0), margin.Position(-300, 70)]
    table.percent("account margin after a broad 10-point drop",
                  account.actual_margin())


if __name__ == "__main__":
    main()
