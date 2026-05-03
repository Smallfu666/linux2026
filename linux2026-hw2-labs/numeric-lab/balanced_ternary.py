#!/usr/bin/env python3
"""Convert integers to and from balanced ternary."""

from __future__ import annotations

import argparse
from dataclasses import dataclass

NEGATIVE = "-"
ZERO = "0"
POSITIVE = "+"
SYMBOLS = {NEGATIVE: -1, ZERO: 0, POSITIVE: 1}


@dataclass(frozen=True)
class ConversionStep:
    value: int
    quotient: int
    remainder: int
    symbol: str


def to_balanced_ternary(value: int) -> str:
    if value == 0:
        return ZERO

    digits: list[int] = []
    n = value
    while n:
        n, remainder = divmod(n, 3)
        if remainder == 2:
            remainder = -1
            n += 1
        digits.append(remainder)

    digit_to_symbol = {-1: NEGATIVE, 0: ZERO, 1: POSITIVE}
    return "".join(digit_to_symbol[digit] for digit in reversed(digits))


def from_balanced_ternary(text: str) -> int:
    total = 0
    for symbol in text:
        total = total * 3 + SYMBOLS[symbol]
    return total


def negate_text(text: str) -> str:
    return "".join(POSITIVE if ch == NEGATIVE else NEGATIVE if ch == POSITIVE else ZERO for ch in text)


def trace_conversion(value: int) -> list[ConversionStep]:
    if value == 0:
        return [ConversionStep(value=0, quotient=0, remainder=0, symbol=ZERO)]

    steps: list[ConversionStep] = []
    n = value
    while n:
        quotient, remainder = divmod(n, 3)
        if remainder == 2:
            remainder = -1
            quotient += 1
        symbol = NEGATIVE if remainder == -1 else POSITIVE if remainder == 1 else ZERO
        steps.append(
            ConversionStep(
                value=n,
                quotient=quotient,
                remainder=remainder,
                symbol=symbol,
            )
        )
        n = quotient
    return steps


def render_trace(steps: list[ConversionStep]) -> str:
    headers = ("value", "quotient", "remainder", "digit")
    rows = [
        (str(step.value), str(step.quotient), str(step.remainder), step.symbol)
        for step in steps
    ]
    widths = [len(title) for title in headers]
    for row in rows:
        widths = [max(width, len(cell)) for width, cell in zip(widths, row)]

    def fmt(row: tuple[str, ...]) -> str:
        return " | ".join(cell.ljust(width) for cell, width in zip(row, widths))

    lines = [fmt(headers), "-+-".join("-" * width for width in widths)]
    lines.extend(fmt(row) for row in rows)
    return "\n".join(lines)


def build_demo(value: int) -> str:
    encoded = to_balanced_ternary(value)
    negated = negate_text(encoded)
    lines = [
        f"value: {value}",
        f"balanced ternary: {encoded}",
        f"round trip: {from_balanced_ternary(encoded)}",
        f"text negation: {negated}",
        f"decoded negation: {from_balanced_ternary(negated)}",
        "",
        render_trace(trace_conversion(value)),
    ]
    return "\n".join(lines)


def build_table(values: list[int]) -> str:
    headers = ("value", "balanced ternary", "negated text", "decoded negation")
    rows = []
    for value in values:
        encoded = to_balanced_ternary(value)
        negated = negate_text(encoded)
        rows.append((str(value), encoded, negated, str(from_balanced_ternary(negated))))

    widths = [len(title) for title in headers]
    for row in rows:
        widths = [max(width, len(cell)) for width, cell in zip(widths, row)]

    def fmt(row: tuple[str, ...]) -> str:
        return " | ".join(cell.ljust(width) for cell, width in zip(row, widths))

    lines = [fmt(headers), "-+-".join("-" * width for width in widths)]
    lines.extend(fmt(row) for row in rows)
    return "\n".join(lines)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Convert integers to balanced ternary."
    )
    parser.add_argument("--value", type=int)
    parser.add_argument(
        "--table",
        nargs="*",
        type=int,
        default=[-10, -5, -2, -1, 0, 1, 2, 5, 10],
        help="Optional list of integers to summarize as a table.",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    if args.value is not None:
        print(build_demo(args.value))
        return
    print(build_table(args.table))


if __name__ == "__main__":
    main()
