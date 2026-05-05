# Experiment B: 3-bit packing / unpacking

This experiment corresponds to quiz6 Problem B: filling the missing pieces in a
manual `pack_indices()` / `unpack_indices()` implementation.

## What It Verifies

The C program implements:

```c
static void pack_indices(const uint8_t *idx, uint8_t *packed, int n, int bits);
static void unpack_indices(const uint8_t *packed, uint8_t *idx, int n, int bits);
```

It does not use C bit-fields. The layout is explicit LSB-first packing:

- `bp >> 3` selects the byte containing bit position `bp`.
- `bp & 7` selects the bit offset inside that byte.
- `(1 << bits) - 1` creates a mask of the low `bits` bits.

## Actual Output

The first test prints the reference case:

```text
case 1: idx={5,3,7}, bits=3, LSB-first
  packed[0] binary: 11011101
  packed[0] hex:    0xDD
  packed[1] binary: 00000001
  packed[1] hex:    0x01
  unpacked:         {5, 3, 7}
```

The second test runs `1000` deterministic randomized roundtrips for each
`bits=1..7`, with `n=128`. The third test prints the masks:

```text
bits=1 -> 0x01
bits=2 -> 0x03
bits=3 -> 0x07
...
bits=7 -> 0x7F
```

## Portability Notes

The packing layout is portable because it uses `uint8_t` storage and explicit
shifts and masks. It does not depend on compiler bit-field allocation order,
endianness of multi-byte integers, or struct padding.

The code uses unsigned constants such as `UINT32_C(1)` while building masks, so
the left shifts used here are unsigned and well-defined for the tested widths.
C bit-fields are intentionally avoided because their allocation order within a
storage unit is implementation-defined and not suitable for a portable packed
byte format.
