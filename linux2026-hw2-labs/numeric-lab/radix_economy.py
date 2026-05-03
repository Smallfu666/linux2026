#!/usr/bin/env python3
"""Compare radix-economy ratios across bases."""

from __future__ import annotations

import argparse
import math


def radix_economy_ratio(base: int) -> float:
    return base / (math.e * math.log(base))


def build_report(bases: list[int]) -> str:
    lines = ["base,E(b)/E(e)"]
    lines.extend(f"{base},{radix_economy_ratio(base):.6f}" for base in bases)
    return "\n".join(lines)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Compare radix-economy ratios for selected bases."
    )
    parser.add_argument("--bases", nargs="*", type=int, default=[2, 3, 4, 8, 10, 16])
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    print(build_report(args.bases))


if __name__ == "__main__":
    main()
