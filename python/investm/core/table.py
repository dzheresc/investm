"""Console output helpers for the worked examples."""

from typing import Sequence


def heading(title: str) -> None:
    print(f"\n=== {title} ===")


def value(label: str, x: float, unit: str = "") -> None:
    print(f"  {label:<46} {x:14.6f} {unit}")


def percent(label: str, fraction: float) -> None:
    print(f"  {label:<46} {100.0 * fraction:13.4f} %")


def table(header: Sequence[str],
          rows: Sequence[Sequence[object]],
          width: int = 12,
          precision: int = 4) -> None:
    """Prints a fixed-width table; floats use `precision` decimals."""

    def fmt(cell: object) -> str:
        if isinstance(cell, float):
            return f"{cell:{width}.{precision}f}"
        return f"{cell!s:>{width}}"

    print(" " + "".join(" " + f"{h:>{width}}" for h in header))
    print("  " + "-" * (len(header) * (width + 1)))
    for row in rows:
        print(" " + "".join(" " + fmt(c) for c in row))
