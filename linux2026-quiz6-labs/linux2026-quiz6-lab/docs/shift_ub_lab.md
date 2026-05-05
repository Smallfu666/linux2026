# Experiment C: signed shift UB

This experiment corresponds to the quiz6 signed-shift pitfall behind expressions
like `1 << 31`.

## What It Verifies

The program evaluates and prints:

```c
int a = 1 << 30;
int b = 1 << 31;
unsigned int c = 1u << 31;
uint32_t d = UINT32_C(1) << 31;
```

It also compares:

```c
uint32_t bit_bad(int n)  { return 1 << n; }
uint32_t bit_good(int n) { return 1u << n; }
```

for `n = 0, 1, 30, 31`.

## Actual Output

On a common two's-complement x86-64 system, the non-sanitized run typically
prints the same bit pattern for the high bit:

```text
int b = 1 << 31          decimal= -2147483648 hex=0x80000000
unsigned c = 1u << 31    decimal=  2147483648 hex=0x80000000
```

That output is observational only. The signed expression is still not portable
C. `make sanitize` runs GCC and Clang UBSan builds and reports runtime errors
for the signed left shift into the sign bit.

## Portability Notes

The literal `1` has type `int`, so `1 << 31` is a signed `int` left shift on
typical 32-bit-`int` targets. C does not define the result when the mathematical
result cannot be represented in the signed type. That makes `1 << 31` undefined
behavior, even if the machine instruction happens to produce `0x80000000`.

`1u << 31` and `UINT32_C(1) << 31` use unsigned arithmetic. For a 32-bit unsigned
type, shifting bit 0 to bit 31 is well-defined. This is why low-level C code,
including Linux kernel-style `BIT(n)` macros, uses unsigned constants or unsigned
long constants when constructing bit masks.
