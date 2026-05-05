# Handoff: linux2026 quiz6 lab

Status: mostly complete and verified.

Created lab directory:

```text
linux2026-quiz6-lab/
```

Implemented files:

```text
linux2026-quiz6-lab/Makefile
linux2026-quiz6-lab/README.md
linux2026-quiz6-lab/docs/summary.md
linux2026-quiz6-lab/src/fp16_lab.c
linux2026-quiz6-lab/docs/fp16_lab.md
linux2026-quiz6-lab/src/bitpack_lab.c
linux2026-quiz6-lab/docs/bitpack_lab.md
linux2026-quiz6-lab/src/shift_ub_lab.c
linux2026-quiz6-lab/docs/shift_ub_lab.md
linux2026-quiz6-lab/src/ewma_lab.c
linux2026-quiz6-lab/docs/ewma_lab.md
linux2026-quiz6-lab/src/tree_cache_lab.c
linux2026-quiz6-lab/docs/tree_cache_lab.md
linux2026-quiz6-lab/results/ewma_results.csv
```

Verified commands already run from `linux2026-quiz6-lab/`:

```sh
make all
make run
make sanitize
make perf
```

Verification results:

- `make all` passed for gcc/clang, O0/O2, and UBSan builds.
- `make run` completed all five experiments.
- `make sanitize` completed. Expected UBSan reports appeared only for `shift_ub_lab.c` signed `1 << 31`.
- `make perf` worked as a wrapper but skipped because `perf` is not installed in this environment.
- `results/ewma_results.csv` exists and has 3201 lines: 1 header + 4 sequences * 4 precisions * 2 weights * 100 steps.

Important observed outputs:

- FP16:
  - `2048.0 -> fp16 0x6800`, decoded `2048`, exact yes.
  - `2049.0 -> fp16 0x6800`, decoded `2048`, exact no.
  - `65504.0 -> fp16 0x7BFF`, exact yes.
  - `65536.0 -> fp16 0x7C00`, decoded `inf`, overflow.
- Bit packing:
  - `{5,3,7}` with `bits=3` produced `packed[0]=0xDD`, `packed[1]=0x01`.
  - randomized roundtrip passed for `bits=1..7`, `n=128`, `1000` trials each.
- Shift UB:
  - non-sanitized run prints `0x80000000` for both bad and good high-bit cases on this machine.
  - UBSan reports: left shift of 1 by 31 places cannot be represented in type `int`.
- EWMA:
  - `precision=0` shows truncation clearly, especially `small1` staying at fixed `0`.
  - higher precision tracks float much better.
- Tree/cache:
  - 1M lookup run completed for N=1K, 16K, 256K, 1M.
  - for N=1M in one `make run`: malloc BST about `667 ns/lookup`, array about `221 ns/lookup`, block32 about `173 ns/lookup`.

Known cleanup / remaining work if continuing:

- Docs are present and usable, but could be polished by pasting more exact stdout snippets from the latest `make run`.
- There are old ad-hoc binaries left by subagents in `linux2026-quiz6-lab/build/fp16_lab`, `build/shift_ub_lab`, and `build/shift_ub_lab_ubsan`. `make clean` removes `build/`, so this is not a functional issue.
- The requested filename was `/hanoff.md`; due sandbox, this handoff was written at the current workspace root:
  `linux2026-quiz6-labs/hanoff.md`.

Next safe command:

```sh
cd /home/wsl/linux2026/linux2026-quiz6-labs/linux2026-quiz6-lab
make clean
make all
make run
make sanitize
```
