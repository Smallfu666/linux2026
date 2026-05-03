# Linux Agent Handoff

Last updated: 2026-05-04

## Purpose

This workspace already contains the four requested basics labs. Development and smoke validation happened on macOS, so the next agent on Linux should focus on:

- rebuilding with GCC on Linux x86_64
- running `perf stat` where relevant
- refreshing result files with Linux-native evidence
- updating status notes, not rewriting the labs from scratch

Do not turn this into a homework answer. Keep it as evidence, scripts, output files, and concise notes.

## Current Environment Gap

Work completed so far was validated locally on macOS with Clang-compatible builds. That means:

- Linux `perf` counters are still missing
- GCC assembly output is still missing
- x86_64-specific code generation differences are still missing

## First Checks On Linux

Run these before touching the labs:

```bash
uname -a
gcc --version
clang --version || true
perf --version
objdump --version || llvm-objdump --version
```

If `perf` is unavailable or blocked by permissions, note that clearly in `SUMMARY.md` instead of guessing.

## Lab 1: UB Exploitation

Directory:

- [ub_exploitation](/Users/nick/Documents/projects/linux2026/linux2026-hw3-labs/labs/basics/ub_exploitation)

Tasks:

1. Rebuild and rerun with GCC.
2. Regenerate all assembly, disassembly, warnings, and runtime logs on Linux.
3. Compare GCC `-O2` against:
   - `-O0`
   - `-O2 -fwrapv`
   - `-O2 -fno-strict-overflow`
4. Check whether the evidence still shows:
   - signed-overflow branch removal
   - post-dereference null-check removal
   - shift warnings and undefined runtime behavior

Commands:

```bash
cd /Users/nick/Documents/projects/linux2026/linux2026-hw3-labs/labs/basics/ub_exploitation
make clean
make CC=gcc
CC=gcc ./run.sh
```

Files to inspect first:

- `results/signed_overflow.O2.s`
- `results/signed_overflow.O2_wrapv.s`
- `results/null_deref.O2.s`
- `results/shift_out_of_range.O2.warnings.txt`
- `results/*.O0_vs_O2.diff.txt`

If GCC behavior differs from the current macOS/Clang evidence, add a short note near the top of `SUMMARY.md`.

## Lab 2: Dispatch Benchmark

Directory:

- [dispatch_benchmark](/Users/nick/Documents/projects/linux2026/linux2026-hw3-labs/labs/basics/dispatch_benchmark)

Tasks:

1. Rebuild with GCC.
2. Smoke-run all modes once.
3. Run `perf stat` for:
   - `bench_switch predictable`
   - `bench_switch random`
   - `bench_cgoto predictable`
   - `bench_cgoto random`
4. Save the `perf stat` stderr output into:
   - `results/perf_switch.txt`
   - `results/perf_cgoto.txt`
5. Keep the benchmark stdout if useful, but the perf counter output is the priority artifact.

Commands:

```bash
cd /Users/nick/Documents/projects/linux2026/linux2026-hw3-labs/labs/basics/dispatch_benchmark
make clean
make CC=gcc
./dispatch_benchmark --pattern all --dispatch all --program-len 32768 --steps 1000000 --reps 1 --csv
PROGRAM_LEN=32768 STEPS=1000000 REPS=10 CSV=1 perf stat -e cycles,instructions,branches,branch-misses,cache-references,cache-misses ./bench_switch predictable 2> results/perf_switch.txt
PROGRAM_LEN=32768 STEPS=1000000 REPS=10 CSV=1 perf stat -e cycles,instructions,branches,branch-misses,cache-references,cache-misses ./bench_switch random 2>> results/perf_switch.txt
PROGRAM_LEN=32768 STEPS=1000000 REPS=10 CSV=1 perf stat -e cycles,instructions,branches,branch-misses,cache-references,cache-misses ./bench_cgoto predictable 2> results/perf_cgoto.txt
PROGRAM_LEN=32768 STEPS=1000000 REPS=10 CSV=1 perf stat -e cycles,instructions,branches,branch-misses,cache-references,cache-misses ./bench_cgoto random 2>> results/perf_cgoto.txt
```

Sanity check:

- `bench_switch` and `bench_cgoto` should produce the same checksum for the same pattern and parameters.

## Lab 3: Tree Cache Locality

Directory:

- [tree_cache_locality](/Users/nick/Documents/projects/linux2026/linux2026-hw3-labs/labs/basics/tree_cache_locality)

Tasks:

1. Rebuild with GCC.
2. Smoke-run both tree types once.
3. Run `perf stat` for `bst` and `btree16`.
4. Save the `perf stat` stderr output into:
   - `results/perf_bst.txt`
   - `results/perf_btree16.txt`

Commands:

```bash
cd /Users/nick/Documents/projects/linux2026/linux2026-hw3-labs/labs/basics/tree_cache_locality
make clean
make CC=gcc
./bench --type=bst --key-count=100000 --lookups=1000000 --reps=1 --csv
./bench --type=btree16 --key-count=100000 --lookups=1000000 --reps=1 --csv
perf stat -e cycles,instructions,cache-references,cache-misses,branches,branch-misses ./bench --type=bst --key-count=100000 --lookups=1000000 --reps=5 --csv 2> results/perf_bst.txt
perf stat -e cycles,instructions,cache-references,cache-misses,branches,branch-misses ./bench --type=btree16 --key-count=100000 --lookups=1000000 --reps=5 --csv 2> results/perf_btree16.txt
```

Sanity check:

- `bst` and `btree16` should produce the same checksum for the same generated query set.
- Do not describe this as Maple Tree itself. Keep calling it a simplified locality model.

## Lab 4: Container Of Layout

Directory:

- [container_of_layout](/Users/nick/Documents/projects/linux2026/linux2026-hw3-labs/labs/basics/container_of_layout)

Tasks:

1. Rebuild with GCC.
2. Rerun both demos.
3. Check whether the packed-wrapper address still makes low-bit tagging unsafe.

Commands:

```bash
cd /Users/nick/Documents/projects/linux2026/linux2026-hw3-labs/labs/basics/container_of_layout
make clean
make CC=gcc
CC=gcc ./run.sh
```

Files to inspect:

- `results/container_of_demo.txt`
- `results/pointer_tagging_demo.txt`

## What To Update After Linux Runs

After finishing the Linux pass:

1. Update [SUMMARY.md](/Users/nick/Documents/projects/linux2026/linux2026-hw3-labs/SUMMARY.md:1)
2. Mark each lab as:
   - rebuilt on Linux
   - perf completed or blocked
   - manual analysis still pending or done
3. If any source changes were required for GCC/Linux compatibility, note them briefly in `SUMMARY.md`.

## Review Standard

Before accepting the Linux run as complete, verify:

- every lab still builds with `-Wall -Wextra`
- benchmark pairs produce matching checksums
- `results/*.txt` placeholders have been replaced by actual Linux-side counter output where applicable
- no README overclaims equivalence with actual Linux kernel internals
