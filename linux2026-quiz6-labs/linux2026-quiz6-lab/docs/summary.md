# Quiz6 Lab Summary

This summary records conclusions from the local experiments. It intentionally
does not provide direct Google Form answers.

## Experiment A: FP32 / FP16

One-line conclusion: FP16 has enough exponent range for 2048 and 65504, but only
10 explicit fraction bits, so spacing grows with the exponent and 2049 rounds to
2048.

Quiz6 concept: IEEE 754 sign/exponent/fraction fields, finite mantissa precision,
rounding, and overflow.

Trap to watch: exponent and fraction fields are not the value by themselves; the
hidden leading 1 and exponent bias matter for normal values.

Candidate inference: in the FP16 binade `[2048, 4096)`, adjacent representable
values are separated by 2, so the first positive integer not exactly represented
is 2049.

## Experiment B: Bit Packing

One-line conclusion: LSB-first packing with 3-bit values stores `{5, 3, 7}` as
byte `0xDD` followed by byte `0x01`, and unpacking recovers the original values.

Quiz6 concept: byte offset, bit offset within a byte, low-bit masks, and avoiding
C bit-fields for portable wire/storage layouts.

Trap to watch: `bp >> 3` is the byte index, while `bp & 7` is the bit offset
inside that byte. Mixing MSB-first and LSB-first conventions changes the bytes.

Candidate inference: the byte-local offset mask is likely `7`; the low-bits mask
subtraction constant is likely `1`; the byte advance is likely `>> 3`.

## Experiment C: Signed Shift UB

One-line conclusion: `1 << 31` may print the expected-looking bit pattern on a
two's-complement machine, but UBSan reports it because signed left shift outside
the representable `int` range is undefined behavior.

Quiz6 concept: integer literal types, signed versus unsigned shifts, undefined
behavior, and kernel-style bit macros.

Trap to watch: `1` has type `int`; use `1u` or `UINT32_C(1)` when the target is a
32-bit bit mask.

Candidate inference: a shift source in code that later shifts to high bits should
use an unsigned constant.

## Experiment D: Fixed-Point EWMA

One-line conclusion: fixed-point EWMA tracks the floating version better as
precision increases; with low precision, repeated right shifts truncate small
increments and bias the average downward.

Quiz6 concept: Linux-style shift arithmetic, `srtt_us` fixed-point scaling, and
EWMA reciprocal weights.

Trap to watch: a power-of-two reciprocal weight makes division a right shift, but
that also means truncation unless enough fractional precision is kept.

Candidate inference: 3 fractional bits corresponds to storing the average scaled
by 8, so `>> 3` recovers the integer microsecond value.

## Experiment E: Lookup Locality

One-line conclusion: pointer-heavy trees can have good asymptotic lookup
complexity while still losing real time to cache misses and branch behavior;
contiguous and block-based layouts improve locality.

Quiz6 concept: red-black tree/VMA lookup background, cache lines, pointer
chasing, and why big-O does not model hardware locality.

Trap to watch: this lab is only a simplified locality demonstration. It is not a
Linux maple tree implementation and should not be described as one.

Candidate inference: the move from old VMA red-black-tree lookup toward maple
tree is about practical lookup/update scalability and locality, not only
abstract tree height.
