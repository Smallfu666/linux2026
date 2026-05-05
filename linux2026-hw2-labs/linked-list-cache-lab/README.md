# Linked List Cache Lab

This lab compares two `middle node` algorithms on linked lists under allocation layouts that stress cache locality differently. It is intended as reproducible experiment scaffolding for GNU/Linux, not as a ready-made homework writeup.

## Layout

- `src/alloc.c`: linked-list builders for each allocation mode.
- `src/middle_fast_slow.c`: fast/slow pointer implementation.
- `src/middle_two_pass.c`: count-then-walk implementation.
- `src/bench.c`: CLI benchmark driver.
- `scripts/run_bench_matrix.sh`: cross-platform elapsed-time matrix runner.
- `scripts/run_perf_stat.sh`: GNU/Linux `perf stat` matrix runner.
- `scripts/run_perf_record.sh`: GNU/Linux `perf record` helper.
- `scripts/run_perf_report.sh`: GNU/Linux `perf report` helper.
- `results/`: generated CSV files and notes.

## Build

```bash
make
```

Use a different compiler if needed:

```bash
make CC=clang
make CC=gcc
```

Default flags are `-O2 -g -Wall -Wextra -Wpedantic`.

## Benchmark CLI

The benchmark binary is `./bench`.

```bash
./bench --algorithm fast_slow --alloc contiguous --n 1000000 --reps 3
./bench --algorithm two_pass --alloc shuffle --n 1000000 --reps 1 --csv --header
./bench --algorithm all --alloc all --n 100000 --reps 2 --csv
```

Supported allocation modes:

- `contiguous`: one flat node array linked sequentially.
- `malloc`: one `malloc()` per node linked in creation order.
- `shuffle`: one `malloc()` per node, then randomized link permutation.
- `page_spread`: nodes placed at `spread-bytes` stride, default `4096`.

Supported algorithms:

- `fast_slow`
- `two_pass`

CSV output fields from `./bench`:

- `algorithm`
- `alloc_mode`
- `n`
- `trial`
- `elapsed_ns`
- `checksum`

`checksum` is a volatile sink guard derived from the middle node so the traversal is not optimized away.

## Local Matrix Run

For a quick functional run on any POSIX host:

```bash
./scripts/run_bench_matrix.sh
```

Useful overrides:

```bash
LENGTHS="10000 100000" TRIALS=5 ./scripts/run_bench_matrix.sh
SPREAD_BYTES=8192 MAX_SPREAD_BYTES=536870912 ./scripts/run_bench_matrix.sh
```

Output:

- `results/local_elapsed.csv`

## GNU/Linux perf stat Run

This script is the main path for the homework-style cache study and expects GNU/Linux with `perf` available.

```bash
./scripts/run_perf_stat.sh
```

Useful overrides:

```bash
LENGTHS="10000 100000 1000000 10000000" TRIALS=5 ./scripts/run_perf_stat.sh
ALLOC_MODES="contiguous malloc shuffle" ./scripts/run_perf_stat.sh
```

Output:

- `results/perf_stat.csv`

CSV fields:

- `algorithm`
- `alloc_mode`
- `n`
- `trial`
- `cycles`
- `instructions`
- `cache_references`
- `cache_misses`
- `branches`
- `branch_misses`
- `llc_loads`
- `llc_load_misses`
- `elapsed_ns`
- `checksum`

The script collects:

- `cycles`
- `instructions`
- `cache-references`
- `cache-misses`
- `branches`
- `branch-misses`
- `LLC-loads`
- `LLC-load-misses`

## GNU/Linux perf record / report

Profile one combination:

```bash
ALGORITHM=fast_slow ALLOC_MODE=shuffle NODES=1000000 ./scripts/run_perf_record.sh
./scripts/run_perf_report.sh
```

For `perf c2c`, run a workload that meaningfully shares cache lines and invoke `perf c2c` manually on a Linux host that supports it.

## WSL/Linux Validation

Validated on 2026-05-04 in WSL2 Linux x86_64 with `gcc 13.3.0`:

- `make clean && make` passed
- `./bench --algorithm all --alloc all --n 1000 --reps 1 --csv --header` passed
- `./scripts/run_bench_matrix.sh` regenerated `results/local_elapsed.csv`
- `perf` is not installed in this WSL image, so the `perf`-based scripts are currently blocked here

The `perf` scripts remain part of the lab because they are expected to run on a GNU/Linux host with `perf` available.

## Interpreting Cache Metrics

Common derived ratios:

- Cache miss rate: `cache_misses / cache_references`
- LLC miss rate: `llc_load_misses / llc_loads`
- Branch miss rate: `branch_misses / branches`
- IPC: `instructions / cycles`

The point of the lab is not just asymptotic complexity. `fast_slow` and `two_pass` are both `O(n)`, but they differ in pointer-walk pattern, pass count, branch behavior, and how much opportunity the hardware prefetcher has to hide latency.

## Experiment Limits

- `page_spread` can consume very large virtual and physical memory once pages are touched; the scripts cap it with `MAX_SPREAD_BYTES`.
- The local macOS run is useful only for functional validation and coarse elapsed time. `perf` collection is intentionally Linux-only.
- Results for very small `n` can be noisy because runtime approaches timer overhead.
- This benchmark times the traversal only. Allocation is rebuilt per trial, but allocation cost is outside the timed region.
- The benchmark is single-threaded and does not attempt to pin CPUs or disable frequency scaling.

## Acceptance Criteria

- `make` builds `./bench` successfully.
- `./bench` supports both algorithms and all four allocation modes.
- `./scripts/run_bench_matrix.sh` writes `results/local_elapsed.csv`.
- `./scripts/run_perf_stat.sh` writes `results/perf_stat.csv` on GNU/Linux with `perf`.
- `./scripts/run_perf_record.sh` records a profile on GNU/Linux.
- The benchmark emits stable CSV columns suitable for later plotting or notebook analysis.
- No homework prose is embedded in the outputs; only reproducible lab scaffolding and notes are included.
