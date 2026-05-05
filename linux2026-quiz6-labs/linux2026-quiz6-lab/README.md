# linux2026 quiz6 lab

Small, rerunnable C experiments for quiz6 representation, packing, undefined
behavior, fixed-point, and cache-locality questions.

This lab is meant to help reason from executable evidence. It does not fill in
the Google Form directly.

## Build

```sh
make all
```

`make all` builds every experiment with:

- `gcc -O0`
- `gcc -O2`
- `clang -O0`
- `clang -O2`
- `gcc -fsanitize=undefined`
- `clang -fsanitize=undefined`

## Run

```sh
make run
```

This runs the `gcc -O2` binaries and prints readable stdout. The EWMA experiment
also writes:

```text
results/ewma_results.csv
```

## Sanitizer

```sh
make sanitize
```

This runs the GCC and Clang UBSan builds. The signed-shift experiment is expected
to report undefined behavior for signed left shift into the sign bit.

## Optional perf

```sh
make perf
```

If Linux `perf` is installed and usable, this runs hardware-counter collection
for `tree_cache_lab`. If not, it prints a skip message.

## Clean

```sh
make clean
```

## Experiments

- `src/fp16_lab.c`, `docs/fp16_lab.md`: FP32/FP16 fields, rounding, overflow,
  and why 2049 is not exactly representable in FP16.
- `src/bitpack_lab.c`, `docs/bitpack_lab.md`: 3-bit LSB-first packing and
  unpacking.
- `src/shift_ub_lab.c`, `docs/shift_ub_lab.md`: signed left shift undefined
  behavior and unsigned alternatives.
- `src/ewma_lab.c`, `docs/ewma_lab.md`: fixed-point EWMA precision and
  truncation bias.
- `src/tree_cache_lab.c`, `docs/tree_cache_lab.md`: pointer-heavy lookup versus
  contiguous and block-based lookup locality.
