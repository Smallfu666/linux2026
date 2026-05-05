# Experiment A: IEEE754 / FP16 representation

This experiment corresponds to quiz6 Problem A: reading IEEE 754 fields and
reasoning about FP16 range, rounding, overflow, and exact integer
representability.

## What It Verifies

`src/fp16_lab.c` prints the FP32 raw bits and field split for:

```text
0.0, 1.0, -1.0, 0.5, 2.0, 2048.0, 2049.0, 65504.0, 65536.0, 1e-8
```

For each value it also converts FP32 to a simplified FP16 representation, decodes
the FP16 bits back to FP32, and prints the absolute error and whether the value
round-tripped exactly.

The converter handles zero, normal values, round-to-nearest-even, overflow to
infinity, NaN canonicalization, and enough subnormal handling for the displayed
boundary cases. The subnormal path is an experiment aid, not a replacement for a
production-quality floating-point conversion library.

## Actual Output

The important lines from `make run` are:

```text
2048.0    ... fp16_raw 0x6800 decoded  2048       exact yes
2049.0    ... fp16_raw 0x6800 decoded  2048       exact no
65504.0   ... fp16_raw 0x7BFF decoded  65504      exact yes
65536.0   ... fp16_raw 0x7C00 decoded  inf        exact no
```

The program also prints:

```text
2048 exact: yes
2049 exact: no
65504 maps to max finite fp16 0x7BFF: yes
65536 overflows to +inf 0x7C00: yes
```

## Why 2049 Is The First Positive Integer Missed

FP16 has 1 sign bit, 5 exponent bits, and 10 explicit fraction bits. Normal
numbers also have an implicit leading `1`, so the significand has 11 bits of
precision.

At values in `[1024, 2048)`, the spacing is `1`, so every integer in that range
is representable. At values in `[2048, 4096)`, the exponent has grown by one, so
the same 11-bit significand precision now spans a binade twice as wide. The
spacing becomes `2`.

Therefore `2048` is exactly representable, but the next representable FP16 value
is `2050`. The integer `2049` is exactly halfway between them and rounds to
`2048` under round-to-nearest-even because the lower candidate has an even low
significand bit.

## Portability Notes

The bit dump uses `memcpy` between `float` and `uint32_t`, which avoids C strict
aliasing issues. Interpreting the result as IEEE 754 binary32 assumes the target
uses the common IEEE 754 representation for `float`; that is true on the tested
Linux x86-64 environment but is not guaranteed by ISO C alone.

The conversion code uses unsigned integer shifts and masks. No undefined signed
shift behavior is required for the FP16 experiment.
