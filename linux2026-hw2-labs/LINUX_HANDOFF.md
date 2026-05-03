# Linux2026 Homework Labs Handoff

Last updated: 2026-05-04

## Purpose

This workspace is for building reproducible lab-style experiments for Linux2026 Homework 2, not for writing the final homework answers directly.

The original request emphasized:

- prioritize the linked-list cache experiment first
- build runnable labs instead of answering the whole homework
- keep the output as evidence, tooling, scripts, and observations
- use subagents in parallel when helpful
- review subagent output before accepting it
- prepare work that can later be moved onto a GNU/Linux environment for deeper measurement

## What Was Requested

The requested direction was:

1. build `linked-list-cache-lab/` first
2. include reproducible C code, build files, perf scripts, CSV-friendly output, and README
3. run a benchmark once and generate a result artifact
4. after that, continue with the next useful lab
5. eventually move the work to Linux for further development and measurement

## Current Workspace State

There are currently two completed lab directories:

- [linked-list-cache-lab](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/linked-list-cache-lab)
- [numeric-lab](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/numeric-lab)

There is no git repository initialized in this directory at the moment.

## Environment Used So Far

Development so far happened on:

- host OS: Darwin arm64
- compiler: Apple clang 17
- Python: 3.14.4

Important environment limits during this phase:

- `perf` was not available locally
- Graphviz `dot` was not available locally
- GNU/Linux-only measurements could not be executed on this host

That means the Linux-specific scripts were prepared, but some of them were only validated structurally, not exercised end-to-end.

## Completed Work

### 1. linked-list-cache-lab

Directory:

- [linked-list-cache-lab](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/linked-list-cache-lab)

Key files:

- [README.md](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/linked-list-cache-lab/README.md:1)
- [Makefile](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/linked-list-cache-lab/Makefile:1)
- [src/list.h](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/linked-list-cache-lab/src/list.h:1)
- [src/alloc.c](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/linked-list-cache-lab/src/alloc.c:1)
- [src/bench.c](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/linked-list-cache-lab/src/bench.c:1)
- [src/middle_fast_slow.c](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/linked-list-cache-lab/src/middle_fast_slow.c:1)
- [src/middle_two_pass.c](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/linked-list-cache-lab/src/middle_two_pass.c:1)
- [scripts/run_bench_matrix.sh](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/linked-list-cache-lab/scripts/run_bench_matrix.sh:1)
- [scripts/run_perf_stat.sh](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/linked-list-cache-lab/scripts/run_perf_stat.sh:1)
- [scripts/run_perf_record.sh](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/linked-list-cache-lab/scripts/run_perf_record.sh:1)
- [scripts/run_perf_report.sh](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/linked-list-cache-lab/scripts/run_perf_report.sh:1)

Implemented capabilities:

- linked list node shape:
  - `struct node { struct node *next; uint64_t payload; };`
- allocation modes:
  - `contiguous`
  - `malloc`
  - `shuffle`
  - `page_spread`
- middle-node algorithms:
  - `fast_slow`
  - `two_pass`
- benchmark CLI:
  - `--algorithm`
  - `--alloc`
  - `--n`
  - `--reps`
  - `--seed`
  - `--spread-bytes`
  - `--csv`
  - `--header`
- anti-optimization guard:
  - checksum + volatile sink
- Linux perf support:
  - `perf stat`
  - `perf record`
  - helper for `perf report`

What was actually verified locally:

- `make` succeeded
- benchmark binary executed successfully
- `--algorithm all --alloc all` smoke test succeeded
- local elapsed-time matrix was run

Generated local artifacts:

- [results/local_elapsed.csv](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/linked-list-cache-lab/results/local_elapsed.csv:1)
- [results/README.md](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/linked-list-cache-lab/results/README.md:1)

Important limitation:

- `run_perf_stat.sh` and `run_perf_record.sh` were not run end-to-end because this host was not GNU/Linux

Observed integration issue that was already handled:

- one subagent introduced a parallel benchmark implementation with separate `main.c` / `list_bench.c`
- that version conflicted with the main implementation and would have caused dual-entry build problems
- those conflicting files were intentionally removed after review

### 2. numeric-lab

Directory:

- [numeric-lab](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/numeric-lab)

Key files:

- [README.md](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/numeric-lab/README.md:1)
- [Makefile](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/numeric-lab/Makefile:1)
- [ieee754_decode.c](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/numeric-lab/ieee754_decode.c:1)
- [fp_assoc.c](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/numeric-lab/fp_assoc.c:1)
- [binary_fraction.py](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/numeric-lab/binary_fraction.py:1)
- [radix_economy.py](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/numeric-lab/radix_economy.py:1)
- [balanced_ternary.py](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/numeric-lab/balanced_ternary.py:1)
- [graphviz/binary_fraction_0_1.dot](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/numeric-lab/graphviz/binary_fraction_0_1.dot:1)
- [graphviz/ieee754_double_0_1.dot](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/numeric-lab/graphviz/ieee754_double_0_1.dot:1)

Implemented capabilities:

- IEEE-754 float/double raw bit decoding for `0.1`, `0.2`, `0.3`, and `0.1 + 0.2`
- floating-point associativity experiments for classic cancellation and scale-gap cases
- decimal `0.1` to binary repeated-doubling expansion with first 80 fractional bits
- radix economy output using `E(b)/E(e) = b / (e ln b)`
- balanced ternary conversion with `+`, `0`, `-`
- text-form negation by swapping `+` and `-`
- Graphviz source for binary-fraction flow and IEEE-754 layout

What was actually verified locally:

- `make` succeeded
- both C binaries ran successfully
- Python scripts passed `python3 -m py_compile`
- sample outputs were generated

Generated local artifacts:

- [results/ieee754_decode.txt](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/numeric-lab/results/ieee754_decode.txt:1)
- [results/fp_assoc.txt](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/numeric-lab/results/fp_assoc.txt:1)
- [results/binary_fraction.txt](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/numeric-lab/results/binary_fraction.txt:1)
- [results/radix_economy.csv](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/numeric-lab/results/radix_economy.csv:1)
- [results/balanced_ternary.csv](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/numeric-lab/results/balanced_ternary.csv:1)
- [results/README.md](/Users/nick/Documents/projects/linux2026/linux2026-hw2-labs/numeric-lab/results/README.md:1)

Observed integration issue that was already handled:

- one subagent changed the Python side toward a different teaching style
- after review, the Python files were aligned back to the original requested experiment goals:
  - `binary_fraction.py` prints 80 bits by default
  - `radix_economy.py` uses the requested formula
  - `balanced_ternary.py` uses `+ / 0 / -` and explicit negation swapping

## Subagent Usage Summary

Subagents were used in parallel on both labs.

How they were used:

- one worker focused on core C implementation
- one worker focused on scripts, docs, Python support, or results scaffolding

How their outputs were handled:

- their code was reviewed before being accepted
- conflicting interfaces were removed or rewritten
- only the parts aligned with the requested lab goals were kept

## What Still Needs To Be Done On Linux

### linked-list-cache-lab

High priority Linux follow-up:

1. run `make` again on GNU/Linux
2. verify `perf` is installed and usable by the current user
3. run `scripts/run_perf_stat.sh`
4. run `scripts/run_perf_record.sh`
5. inspect output with `scripts/run_perf_report.sh`
6. optionally try `perf c2c` if the target Linux host supports it
7. decide realistic upper bounds for `page_spread` and large `n`, especially `1e7` and `1e8`
8. if needed, pin CPUs or control frequency noise for cleaner measurements

Suggested Linux validation commands:

```bash
cd linked-list-cache-lab
make clean && make
./bench --algorithm all --alloc all --n 1000 --reps 1 --csv --header
TRIALS=5 LENGTHS="10000 100000 1000000 10000000" ./scripts/run_perf_stat.sh
ALGORITHM=fast_slow ALLOC_MODE=shuffle NODES=1000000 ./scripts/run_perf_record.sh
./scripts/run_perf_report.sh
```

### numeric-lab

Linux follow-up is lighter:

1. rebuild and rerun sample outputs
2. install Graphviz if image rendering is desired
3. render the `.dot` files into PNG or SVG if needed for the homework materials

Suggested Linux validation commands:

```bash
cd numeric-lab
make clean && make
./ieee754_decode
./fp_assoc
python3 binary_fraction.py
python3 radix_economy.py
python3 balanced_ternary.py
dot -Tpng graphviz/binary_fraction_0_1.dot -o results/binary_fraction_0_1.png
dot -Tpng graphviz/ieee754_double_0_1.dot -o results/ieee754_double_0_1.png
```

## Recommended Next Lab

The next recommended lab is `memory-lab`.

Reason:

- it is still strongly aligned with Homework 2
- it can be done mostly in user space
- it benefits from Linux for `/proc`, page-fault observation, and `perf`
- it naturally complements the current two labs

Recommended scope for `memory-lab`:

- `malloc_overcommit.c`
- `page_fault_touch.c`
- `alignment_bench.c`
- `pointer_tagging_demo.c`
- `void_pointer_tests.c`
- Linux-friendly scripts for `perf stat` and `/usr/bin/time -v`

## Notes For The Next Agent On Linux

- treat these labs as experiment infrastructure, not final prose
- preserve the existing directory names and CLI surfaces unless there is a strong reason to change them
- for linked-list work, do not remove the current `bench` CSV interface because the scripts depend on it
- if `page_spread` becomes too large on Linux, prefer bounding it in scripts rather than weakening the allocator model
- if `perf` event names differ across kernels or hardware, adapt the scripts carefully and document the change in the relevant README
- if git is initialized later, keep the two labs as separate logical commits if possible

## Quick Resume Summary

If resuming on Linux, start with this sequence:

1. validate `linked-list-cache-lab` build
2. run real `perf stat` collection
3. run one `perf record` profile and inspect it
4. render numeric Graphviz diagrams if needed
5. start `memory-lab`
