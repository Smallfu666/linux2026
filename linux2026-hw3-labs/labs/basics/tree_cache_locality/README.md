# Tree Cache Locality

This lab compares two lookup-heavy tree layouts:

- `bst`: a balanced binary-search-tree shape built from individually allocated nodes.
- `btree16`: a simplified fanout-16 B-tree-like structure over the same sorted keys.

The point is to study locality, pointer chasing, and depth, not to reproduce Linux kernel internals.

## Layout

- `src/tree.c`: builders, destructors, and lookup functions for both structures.
- `src/bench.c`: CLI benchmark driver and CSV output.
- `src/timing.c`: portable monotonic timer helper.
- `run.sh`: smoke-run helper that writes `results/local_elapsed.csv`.
- `results/`: output placeholder directory.

## Build

```bash
make
```

You can also choose the compiler explicitly:

```bash
make CC=clang
make CC=gcc
```

The build uses `-Wall -Wextra` and `-std=gnu11`.

## Benchmark Usage

```bash
./tree_cache_locality --mode bst --key-count 65536 --lookups 262144 --reps 5
./tree_cache_locality --mode btree16 --key-count 65536 --lookups 262144 --reps 5
./tree_cache_locality --mode all --csv
./bench --type=bst --key-count 100000 --lookups 1000000 --reps 5 --csv
./bench --type=btree16 --key-count 100000 --lookups 1000000 --reps 5 --csv
```

Flags:

- `--mode bst|btree16|all`
- `--type bst|btree16|all`
- `--key-count N`
- `--lookups N`
- `--reps N`
- `--seed N`
- `--csv`

The lookup stream is generated once from the key set and reused across both structures so the benchmark compares traversal cost, not query generation.

## Locality Caveats

The `bst` variant is deliberately scattered by allocating each node separately and building the tree from a randomized allocation order. That makes cache misses and pointer chasing much more visible than a compact array-backed representation.

The `btree16` variant is a simplified model of a high-fanout search tree. It reduces depth and groups keys into cache-friendlier nodes, but it is not Maple Tree:

- no range semantics
- no RCU or concurrency behavior
- no gap tracking
- no kernel-style update or rebalance logic

That simplification is intentional. The goal is to make locality effects easy to observe in userspace, not to claim functional equivalence with the kernel data structure.

## Linux perf Usage

On GNU/Linux, run the benchmark under `perf stat` to capture cache and branch behavior:

```bash
perf stat -e cycles,instructions,cache-references,cache-misses,branches,branch-misses ./bench --type=bst --key-count 100000 --lookups 1000000 --reps 5 --csv
perf stat -e cycles,instructions,cache-references,cache-misses,branches,branch-misses ./bench --type=btree16 --key-count 100000 --lookups 1000000 --reps 5 --csv
```

Useful counters:

- `cycles`
- `instructions`
- `branches`
- `branch-misses`
- Save Linux-side outputs into `results/perf_bst.txt` and `results/perf_btree16.txt`.
- `cache-references`
- `cache-misses`

## Notes

- This is a benchmark scaffold, not a published result.
- The outputs under `results/` stay lightweight so you can rerun and collect fresh measurements later.
