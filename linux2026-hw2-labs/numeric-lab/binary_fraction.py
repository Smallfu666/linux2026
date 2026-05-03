#!/usr/bin/env python3
"""Show how decimal 0.1 expands in binary."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
from fractions import Fraction


@dataclass(frozen=True)
class Step:
    index: int
    remainder: Fraction
    doubled: Fraction
    bit: int
    next_remainder: Fraction


def format_fraction(value: Fraction) -> str:
    if value.denominator == 1:
        return str(value.numerator)
    return f"{value.numerator}/{value.denominator}"


def serial_bits(value: Fraction, count: int) -> tuple[str, list[Step]]:
    bits: list[str] = []
    steps: list[Step] = []
    remainder = value

    for index in range(1, count + 1):
        doubled = remainder * 2
        bit = 1 if doubled >= 1 else 0
        next_remainder = doubled - bit
        bits.append(str(bit))
        steps.append(
            Step(
                index=index,
                remainder=remainder,
                doubled=doubled,
                bit=bit,
                next_remainder=next_remainder,
            )
        )
        remainder = next_remainder

    return "".join(bits), steps


def canonical_repeat(value: Fraction, limit: int) -> tuple[str, str | None]:
    remainder = value
    seen: dict[Fraction, int] = {}
    bits: list[str] = []

    while remainder and len(bits) < limit:
        if remainder in seen:
            start = seen[remainder]
            return "".join(bits[:start]), "".join(bits[start:])
        seen[remainder] = len(bits)
        doubled = remainder * 2
        bit = 1 if doubled >= 1 else 0
        remainder = doubled - bit
        bits.append(str(bit))

    return "".join(bits), None


def render_table(steps: list[Step]) -> str:
    headers = ("step", "remainder", "x2", "bit", "next")
    rows = [
        (
            str(step.index),
            format_fraction(step.remainder),
            format_fraction(step.doubled),
            str(step.bit),
            format_fraction(step.next_remainder),
        )
        for step in steps
    ]
    widths = [len(title) for title in headers]
    for row in rows:
        widths = [max(width, len(cell)) for width, cell in zip(widths, row)]

    def fmt_row(row: tuple[str, ...]) -> str:
        return " | ".join(cell.ljust(width) for cell, width in zip(row, widths))

    lines = [fmt_row(headers), "-+-".join("-" * width for width in widths)]
    lines.extend(fmt_row(row) for row in rows)
    return "\n".join(lines)


def chunk_bits(bits: str, width: int = 4) -> str:
    return " ".join(bits[i:i + width] for i in range(0, len(bits), width))


def build_demo(numerator: int, denominator: int, bits: int) -> str:
    value = Fraction(numerator, denominator)
    if not (0 <= value < 1):
        raise ValueError("demo expects a fractional value in [0, 1)")

    first_bits, steps = serial_bits(value, bits)
    prefix, repeat = canonical_repeat(value, bits)

    lines = [
        f"{format_fraction(value)} decimal expansion in binary",
        f"binary form: 0.{prefix}({repeat})" if repeat else f"binary form: 0.{prefix}",
        f"first {bits} fractional bits:",
        chunk_bits(first_bits),
        "",
        "first 16 repeated-doubling steps:",
        render_table(steps[:16]),
    ]
    if repeat:
        lines.extend(
            [
                "",
                f"repeat bits: {repeat}",
                "The repeating remainder is why 0.1 is non-terminating in base 2 and must be rounded in IEEE-754.",
            ]
        )
    return "\n".join(lines)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Demonstrate the binary expansion of decimal 0.1."
    )
    parser.add_argument("--numerator", type=int, default=1)
    parser.add_argument("--denominator", type=int, default=10)
    parser.add_argument("--bits", type=int, default=80)
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    print(build_demo(args.numerator, args.denominator, args.bits))


if __name__ == "__main__":
    main()
