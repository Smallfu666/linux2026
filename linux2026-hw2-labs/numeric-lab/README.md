# Numeric Lab

This lab collects small C and Python experiments for IEEE-754 representation, decimal-to-binary expansion, floating-point non-associativity, radix economy, and balanced ternary. It is intended as raw evidence and runnable experiments, not as a finished homework answer.

## Files

- `ieee754_decode.c`: dumps raw float/double bit patterns for `0.1`, `0.2`, `0.3`, and `0.1+0.2`.
- `fp_assoc.c`: compares `(a+b)+c` and `a+(b+c)` for several rounding-sensitive inputs.
- `binary_fraction.py`: prints the first 80 fractional binary bits of decimal `0.1` and shows the repeating block.
- `radix_economy.py`: compares `E(b)/E(e) = b / (e ln b)` across selected bases.
- `balanced_ternary.py`: converts signed integers to balanced ternary using `+`, `0`, `-`, and shows negation by swapping `+` and `-`.
- `graphviz/binary_fraction_0_1.dot`: stepwise decimal-to-binary expansion flow for `0.1`.
- `graphviz/ieee754_double_0_1.dot`: IEEE-754 double layout for `0.1`.
- `results/`: optional sample outputs generated from this host.

## Build

```bash
make
```

The C programs build to:

- `./ieee754_decode`
- `./fp_assoc`

## Run

```bash
./ieee754_decode
./fp_assoc
python3 binary_fraction.py
python3 radix_economy.py
python3 balanced_ternary.py
python3 balanced_ternary.py --value 10
```

If Graphviz is installed on the target host:

```bash
dot -Tpng graphviz/binary_fraction_0_1.dot -o results/binary_fraction_0_1.png
dot -Tpng graphviz/ieee754_double_0_1.dot -o results/ieee754_double_0_1.png
```

## What To Look For

- `ieee754_decode`: `0.1 + 0.2` and `0.3` have different bit patterns in binary floating point.
- `fp_assoc`: equal real-number expressions can round differently depending on grouping.
- `binary_fraction.py`: decimal `0.1` becomes a repeating binary fraction, so finite-width storage must round it.
- `radix_economy.py`: base 3 should come out near the optimum in the classic radix-economy formula.
- `balanced_ternary.py`: negation is structurally simple because digits are symmetric around zero and text negation can be done by swapping `+` and `-`.

## Limits

- These programs show concrete machine behavior on this host; they do not replace reading the C standard or IEEE-754 text.
- The `dot` files are included even if Graphviz is not installed locally.
- Sample outputs in `results/` are illustrative and should be regenerated on the machine used for your writeup.

## WSL/Linux Validation

Validated on 2026-05-04 in WSL2 Linux x86_64 with `gcc 13.3.0`:

- `make clean && make` passed
- `./ieee754_decode` and `./fp_assoc` passed
- `python3 binary_fraction.py`, `python3 radix_economy.py`, and `python3 balanced_ternary.py` were regenerated into `results/`
- `dot` is not installed in this WSL image, so Graphviz rendering of the `.dot` files was not validated here
