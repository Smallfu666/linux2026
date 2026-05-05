# Linux2026 HW3 Labs Summary

Last updated: 2026-05-04

This workspace contains four userspace mini-labs for the Linux2026 basics homework. The goal is to generate reproducible evidence, not to write the final homework answer directly.

## Current Status

- Linux pass completed on WSL2 x86_64 with `gcc 13.3.0`.
- `perf` is unavailable in this image (`perf: command not found`), so benchmark perf artifacts are blocker notes rather than captured counter output.
- No GCC/Linux source compatibility fixes were required in the owned labs.

### 1. `labs/basics/ub_exploitation/`

- Purpose: show how the compiler exploits undefined behavior in signed overflow, null dereference, and out-of-range shifts.
- Key files:
  - [README.md](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/ub_exploitation/README.md:1)
  - [Makefile](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/ub_exploitation/Makefile:1)
  - [run.sh](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/ub_exploitation/run.sh:1)
  - [signed_overflow.c](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/ub_exploitation/signed_overflow.c:1)
  - [null_deref.c](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/ub_exploitation/null_deref.c:1)
  - [shift_out_of_range.c](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/ub_exploitation/shift_out_of_range.c:1)
- Linux status on WSL2 x86_64: rebuilt with GCC and reran `run.sh`; assembly, disassembly, warning logs, runtime logs, and diff files were regenerated in `results/`.
- Useful generated evidence:
  - [results/signed_overflow.O2.s](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/ub_exploitation/results/signed_overflow.O2.s:1)
  - [results/signed_overflow.O2_wrapv.s](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/ub_exploitation/results/signed_overflow.O2_wrapv.s:1)
  - [results/null_deref.O2.s](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/ub_exploitation/results/null_deref.O2.s:1)
  - [results/shift_out_of_range.O2.warnings.txt](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/ub_exploitation/results/shift_out_of_range.O2.warnings.txt:1)
- No source changes were required for GCC/Linux compatibility.

### 2. `labs/basics/dispatch_benchmark/`

- Purpose: compare switch dispatch and computed goto dispatch in a userspace bytecode interpreter.
- Key files:
  - [README.md](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/dispatch_benchmark/README.md:1)
  - [Makefile](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/dispatch_benchmark/Makefile:1)
  - [run.sh](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/dispatch_benchmark/run.sh:1)
  - [src/bench.c](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/dispatch_benchmark/src/bench.c:1)
  - [src/interpreter.c](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/dispatch_benchmark/src/interpreter.c:1)
  - [src/bench_switch.c](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/dispatch_benchmark/src/bench_switch.c:1)
  - [src/bench_cgoto.c](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/dispatch_benchmark/src/bench_cgoto.c:1)
- Linux status on WSL2 x86_64: rebuilt with GCC, smoke-ran all modes, and verified that `bench_switch` and `bench_cgoto` produce matching checksums for the same parameters.
- Useful generated evidence:
  - [results/local_elapsed.csv](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/dispatch_benchmark/results/local_elapsed.csv:1)
  - [results/perf_switch.txt](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/dispatch_benchmark/results/perf_switch.txt:1)
  - [results/perf_cgoto.txt](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/dispatch_benchmark/results/perf_cgoto.txt:1)
- `perf` could not be collected on this WSL image, so the perf result files contain blocker notes instead of hardware counters.

### 3. `labs/basics/tree_cache_locality/`

- Purpose: compare pointer-heavy BST lookup against fanout-16 B-tree lookup as a locality experiment.
- Key files:
  - [README.md](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/tree_cache_locality/README.md:1)
  - [Makefile](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/tree_cache_locality/Makefile:1)
  - [run.sh](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/tree_cache_locality/run.sh:1)
  - [src/bench.c](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/tree_cache_locality/src/bench.c:1)
  - [src/tree.c](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/tree_cache_locality/src/tree.c:1)
- Linux status on WSL2 x86_64: rebuilt with GCC, smoke-ran both tree types, and verified that `bst` and `btree16` produce matching checksums for the same generated query set.
- Useful generated evidence:
  - [results/local_elapsed.csv](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/tree_cache_locality/results/local_elapsed.csv:1)
  - [results/perf_bst.txt](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/tree_cache_locality/results/perf_bst.txt:1)
  - [results/perf_btree16.txt](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/tree_cache_locality/results/perf_btree16.txt:1)
- `perf` could not be collected on this WSL image, so the perf result files contain blocker notes instead of hardware counters.

### 4. `labs/basics/container_of_layout/`

- Purpose: show `container_of`, struct embedding, and why pointer tagging depends on alignment.
- Key files:
  - [README.md](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/container_of_layout/README.md:1)
  - [Makefile](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/container_of_layout/Makefile:1)
  - [run.sh](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/container_of_layout/run.sh:1)
  - [container_of_demo.c](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/container_of_layout/container_of_demo.c:1)
  - [pointer_tagging_demo.c](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/container_of_layout/pointer_tagging_demo.c:1)
- Linux status on WSL2 x86_64: rebuilt with GCC and reran both demos; the output files now reflect the Linux run.
- Useful generated evidence:
  - [results/container_of_demo.txt](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/container_of_layout/results/container_of_demo.txt:1)
  - [results/pointer_tagging_demo.txt](/home/wsl/linux2026/linux2026-hw3-labs/labs/basics/container_of_layout/results/pointer_tagging_demo.txt:1)
- No source changes were required for GCC/Linux compatibility.

## What Still Needs Manual Analysis

- Turn the raw evidence into HackMD prose:
  - explain which branch or compare disappeared in UB cases
  - explain why `-fwrapv` and `-fno-strict-overflow` help signed overflow but not shift UB
  - explain why computed goto may help branch prediction
  - explain why B-tree-style fanout improves locality without claiming it is Maple Tree
  - explain `container_of` and pointer tagging with memory-layout diagrams
- Re-run benchmark-heavy labs on Linux x86_64 with `perf`.
- Decide whether to keep or clean local build artifacts before committing.
