# Experiment E: pointer tree vs contiguous layout lookup

This experiment corresponds to the quiz6 red-black tree / VMA lookup / maple
tree background. It is a cache-locality microbenchmark, not a Linux maple tree
implementation.

## What It Verifies

The program builds three lookup structures for `N = 1K, 16K, 256K, 1M`:

- `malloc_bst`: pointer-based binary search tree, one `malloc` per node.
- `array`: sorted contiguous key/value arrays with binary search.
- `block32`: a simplified block layout with 32 sorted keys per block and a
  contiguous top-level max-key index.

Each size performs 1,000,000 random successful lookups and prints:

- build time
- lookup time
- average ns per lookup
- approximate memory usage

## Actual Output

The exact numbers depend on CPU, memory, compiler, and machine load. A typical
run prints one block per `N`, for example:

```text
N=262144 lookups=1000000
  malloc_bst   build_ms=... lookup_ms=... ns_lookup=... approx_mem=...
  array        build_ms=... lookup_ms=... ns_lookup=... approx_mem=...
  block32      build_ms=... lookup_ms=... ns_lookup=... approx_mem=...
```

On this environment `perf` was not installed (`perf: command not found`), so the
report relies on wall-clock timing. The Makefile still provides `make perf`; it
skips with a message when `perf` is unavailable.

## Portability Notes

The benchmark uses C11 `timespec_get()` for timing and ordinary heap allocation.
It is portable as a C program, but microbenchmark timings are inherently
compiler-, CPU-, cache-, and load-dependent.

## Interpretation

Pointer-based trees have good asymptotic lookup complexity, but each tree step
loads a node that may be far away in memory. That pointer chasing can cause more
cache misses and branch misses than a contiguous layout.

Sorted arrays have binary search branches too, but the data is packed tightly.
The block layout keeps a small top-level index and then scans a compact block,
which demonstrates why cache-friendly multi-key nodes can beat one-key
pointer-heavy nodes in practice.

Linux VMA lookup moved away from the old red-black-tree representation toward
maple tree in Linux 6.1-era kernels for scalability and locality reasons. This
lab only demonstrates the hardware intuition: big-O is not the whole performance
model when cache lines, pointer chasing, and contention matter.
