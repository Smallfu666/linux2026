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
