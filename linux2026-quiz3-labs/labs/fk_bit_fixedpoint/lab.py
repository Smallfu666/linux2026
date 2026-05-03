#!/usr/bin/env python3

from __future__ import annotations

import argparse
import csv
from dataclasses import dataclass
from fractions import Fraction
from pathlib import Path
from textwrap import dedent


U32_MASK = (1 << 32) - 1
INT32_MIN = -(1 << 31)
INT32_MAX = (1 << 31) - 1
PELT_PERIOD_US = 1024
EWMA_SHIFT = 3  # alpha = 1/8


def u32(value: int) -> int:
    return value & U32_MASK


def s32(value: int) -> int:
    value &= U32_MASK
    return value - (1 << 32) if value & (1 << 31) else value


def bit_case(n: int) -> dict[str, str]:
    if n < 0 or n >= 64:
        return {
            "n": str(n),
            "valid": "no",
            "value_hex": "invalid",
            "note": "out of range for a 64-bit BIT() experiment",
        }
    value = 1 << n
    return {
        "n": str(n),
        "valid": "yes",
        "value_hex": f"0x{value:016x}",
        "note": "exact power of two",
    }


def genmask_case(h: int, l: int) -> dict[str, str]:
    if h < 0 or l < 0 or h >= 64 or l >= 64 or h < l:
        return {
            "h": str(h),
            "l": str(l),
            "valid": "no",
            "value_hex": "invalid",
            "note": "invalid bit range",
        }
    width = h - l + 1
    value = ((1 << width) - 1) << l
    return {
        "h": str(h),
        "l": str(l),
        "valid": "yes",
        "value_hex": f"0x{value:016x}",
        "note": "inclusive high/low mask",
    }


def time_after(a: int, b: int) -> bool:
    return s32(u32(b - a)) < 0


def ewma_series(samples: list[int], shift: int, warmup: bool) -> list[Fraction]:
    out: list[Fraction] = []
    value = Fraction(0, 1)
    for idx, sample in enumerate(samples):
        sample_f = Fraction(sample, 1)
        if warmup and idx == 0:
            value = sample_f
        else:
            value = value + (sample_f - value) / (1 << shift)
        out.append(value)
    return out


def fmt_fraction(value: Fraction) -> str:
    return f"{float(value):.6f}"


def write_csv(path: Path, rows: list[dict[str, str]], fieldnames: list[str]) -> None:
    with path.open("w", newline="") as f:
        writer = csv.DictWriter(f, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(rows)


def generate(out_dir: Path) -> None:
    out_dir.mkdir(parents=True, exist_ok=True)

    bit_rows = [bit_case(n) for n in [0, 1, 8, 31, 32, 63, 64]]
    genmask_rows = [genmask_case(h, l) for h, l in [(63, 0), (63, 63), (31, 0), (31, 32), (15, 8)]]

    write_csv(
        out_dir / "bit_cases.csv",
        bit_rows,
        ["n", "valid", "value_hex", "note"],
    )
    write_csv(
        out_dir / "genmask_cases.csv",
        genmask_rows,
        ["h", "l", "valid", "value_hex", "note"],
    )

    abs_rows = []
    for value in [-5, -1, 0, 1, INT32_MAX, INT32_MIN]:
        math_abs = abs(value)
        fits_int32 = math_abs <= INT32_MAX
        abs_rows.append(
            {
                "input": str(value),
                "input_hex": f"0x{u32(value):08x}",
                "mathematical_abs": str(math_abs),
                "fits_int32": "yes" if fits_int32 else "no",
                "safe_abs_i64": str(math_abs),
                "note": (
                    "representable as int32"
                    if fits_int32
                    else "needs wider type; C abs(int) is undefined here"
                ),
            }
        )
    write_csv(
        out_dir / "abs_int_min_cases.csv",
        abs_rows,
        ["input", "input_hex", "mathematical_abs", "fits_int32", "safe_abs_i64", "note"],
    )

    time_rows: list[dict[str, str]] = []
    for delta in [INT32_MAX, 1 << 31]:
        a = delta
        b = 0
        after_ab = time_after(a, b)
        after_ba = time_after(b, a)
        time_rows.append(
            {
                "delta": str(delta),
                "a_hex": f"0x{u32(a):08x}",
                "b_hex": f"0x{u32(b):08x}",
                "time_after(a,b)": str(after_ab).lower(),
                "time_after(b,a)": str(after_ba).lower(),
                "note": (
                    "half-range-safe ordering"
                    if delta == INT32_MAX
                    else "exactly 2^31 apart; ordering becomes ambiguous"
                ),
            }
        )
    write_csv(
        out_dir / "time_after_boundary.csv",
        time_rows,
        ["delta", "a_hex", "b_hex", "time_after(a,b)", "time_after(b,a)", "note"],
    )

    samples = [100] * 8
    ewma_no_warmup = ewma_series(samples, EWMA_SHIFT, warmup=False)
    ewma_warmup = ewma_series(samples, EWMA_SHIFT, warmup=True)
    ewma_rows = []
    for idx, sample in enumerate(samples, start=1):
        ewma_rows.append(
            {
                "step": str(idx),
                "sample": str(sample),
                "no_warmup": fmt_fraction(ewma_no_warmup[idx - 1]),
                "with_warmup": fmt_fraction(ewma_warmup[idx - 1]),
            }
        )
    write_csv(
        out_dir / "ewma_series.csv",
        ewma_rows,
        ["step", "sample", "no_warmup", "with_warmup"],
    )

    checkpoints = [1, 2, 3, 4, 8, 16, 128, 1024, 2048, 1_000_000]
    pelt_rows: list[dict[str, str]] = []
    for limit in checkpoints:
        compensated_total = 0
        compensated_carry = 0
        uncompensated_total = 0
        uncompensated_carry = 0
        for _ in range(limit):
            compensated_carry += 1
            compensated_total += compensated_carry // PELT_PERIOD_US
            compensated_carry %= PELT_PERIOD_US

            uncompensated_total += 1 // PELT_PERIOD_US
            uncompensated_carry = 0

        pelt_rows.append(
            {
                "updates": str(limit),
                "elapsed_us": str(limit),
                "compensated_periods_passed": str(compensated_total),
                "compensated_remainder_us": str(compensated_carry),
                "uncompensated_periods_passed": str(uncompensated_total),
                "uncompensated_remainder_us": str(uncompensated_carry),
            }
        )
    write_csv(
        out_dir / "pelt_period_contrib.csv",
        pelt_rows,
        [
            "updates",
            "elapsed_us",
            "compensated_periods_passed",
            "compensated_remainder_us",
            "uncompensated_periods_passed",
            "uncompensated_remainder_us",
        ],
    )

    abs_text = dedent(
        f"""
        abs(INT_MIN) boundary note
        - INT_MIN = {INT32_MIN}
        - INT_MAX = {INT32_MAX}
        - On two's-complement hardware, {-INT32_MIN} is not representable as int32.
        - `out/abs_int_min_cases.csv` shows that ordinary inputs still have a representable int32 absolute value, but `INT_MIN` does not.
        - In C, `abs(INT_MIN)` therefore has undefined behavior unless you widen first.
        """
    ).strip()
    (out_dir / "abs_int_min.txt").write_text(abs_text + "\n")

    boundary_md = dedent(
        f"""
        # Boundary Cases

        This lab exercises the kernel-style edge cases from quiz 3 in normal user space.

        ## BIT and GENMASK

        - `BIT(n)` is valid for `0 <= n < 64` in this 64-bit lab.
        - `BIT(63)` is the top bit, `0x8000000000000000`.
        - `BIT(64)` is out of range and marked invalid in the table below.
        - `GENMASK(h, l)` is inclusive on both ends.
        - `GENMASK(31, 32)` is invalid because the high bit is below the low bit.

        See `out/bit_cases.csv` and `out/genmask_cases.csv`.

        ## abs(INT_MIN)

        `abs(INT_MIN)` is a boundary failure because the positive magnitude of
        `-2147483648` does not fit in a signed 32-bit `int`. The C standard leaves
        that case undefined. The generated table makes the failure visible by
        comparing the mathematical absolute value with whether it still fits
        in signed 32-bit.

        See `out/abs_int_min.txt` and `out/abs_int_min_cases.csv`.

        ## time_after(a, b)

        The Linux wraparound comparison works as long as the timestamps are less
        than `2^31` apart. At exactly `2^31`, the ordering becomes ambiguous.

        | delta | time_after(a,b) | time_after(b,a) | result |
        | --- | --- | --- | --- |
        | `2^31 - 1` | true | false | safe, half-range still ordered |
        | `2^31` | true | true | ambiguous, both directions claim success |

        See `out/time_after_boundary.csv`.

        ## EWMA warm-up

        The warm-up special case seeds the running average with the first sample.
        Without it, the estimate starts from zero and is biased low for the first
        few observations.

        This lab uses a constant sample stream of `100` and `alpha = 1/8`.
        The first row already shows the difference:

        - no warm-up: `12.500000`
        - warm-up: `100.000000`

        See `out/ewma_series.csv`.

        ## PELT-style period_contrib

        The PELT experiment uses `delta = 1 us`, `N = 1e6`, and a `1024 us`
        period. The compensated counter carries fractional progress across calls,
        while the uncompensated version truncates every 1 us update to zero.

        Final totals after `1e6` updates:

        - compensated: `976` full periods, `576 us` remainder
        - uncompensated: `0` full periods, `0 us` remainder

        See `out/pelt_period_contrib.csv`.
        """
    ).strip()
    (out_dir.parent / "boundary_cases.md").write_text(boundary_md + "\n")


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out-dir", default="out")
    args = parser.parse_args()
    generate(Path(args.out_dir))


if __name__ == "__main__":
    main()
