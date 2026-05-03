# linux2026-quiz3-labs

User-space mini labs for understanding selected topics from the HackMD Linux 2026 quiz 3 set.

## Quick Start

Generate all labs:

```sh
make all
```

Regenerate one lab:

```sh
make a_pointer_layout
make bc_list_hlist
make e_hash
make fk_bit_fixedpoint
make ch_sort_trace
```

Clean all generated artifacts:

```sh
make clean
```

## Lab Map

| Lab | HackMD problems | Entry | Key outputs |
| --- | --- | --- | --- |
| `labs/a_pointer_layout` | A | `make a_pointer_layout` | `compile_matrix.md`, `artifacts/*_stdout.txt` |
| `labs/bc_list_hlist` | B/C | `make bc_list_hlist` | `artifacts/delete_*.dot`, `artifacts/iteration.txt`, `artifacts/summary.txt` |
| `labs/e_hash` | E | `make e_hash` | `out/hash_stats.csv`, `out/hash_summary.txt`, `out/hist_*.png`, `out/avalanche_heatmap.png` |
| `labs/fk_bit_fixedpoint` | F/K | `make fk_bit_fixedpoint` | `boundary_cases.md`, `out/*.csv`, `out/abs_int_min.txt`, `out/abs_int_min_cases.csv` |
| `labs/ch_sort_trace` | C/H | `make ch_sort_trace` | `artifacts/list_sort_trace_n13.txt`, `artifacts/heapsort_comparisons.csv` |

## Conclusions

### A. Pointer/Layout

Files:
- `labs/a_pointer_layout/compile_matrix.md`
- `labs/a_pointer_layout/artifacts/portable_stdout.txt`
- `labs/a_pointer_layout/artifacts/gnu_stdout.txt`
- `labs/a_pointer_layout/artifacts/c11_stdout.txt`

Observation:
- `sizeof(a)` is the full array size, while `sizeof(p)` is just pointer size after decay.
- `&a + 1` and `p + 5` land on the same address in this example, but the types differ: one advances by one whole array object, the other by five elements.
- `offsetof` plus byte subtraction reconstructs the outer object exactly as `container_of` does.
- In the compile matrix, the C11-only variant fails under `-std=c99` exactly because `_Static_assert` and `_Generic` are not available there.

Exam takeaway:
- Distinguish array object type from decayed pointer type.
- `container_of` is just pointer rebasing with stronger compile-time checks layered on top.

### B/C. list and hlist

Files:
- `labs/bc_list_hlist/artifacts/delete_head.dot`
- `labs/bc_list_hlist/artifacts/delete_middle.dot`
- `labs/bc_list_hlist/artifacts/delete_tail.dot`
- `labs/bc_list_hlist/artifacts/iteration.txt`

Observation:
- Indirect-pointer delete works because `pp` stores the address of the link that currently points at the node being examined, so deleting head/middle/tail is the same assignment through `*pp`.
- The hlist-style `pprev` field records exactly that predecessor link address.
- Safe iteration caches `next` before unlinking; unsafe iteration touches `pos->next` after `list_del()` and steps onto the poison pointer.

Exam takeaway:
- `probe_node_t **pp` and `hlist_node.pprev` solve the same problem: update the predecessor's link without a special head case.
- `list_for_each_entry_safe()` exists because delete invalidates the current node's next/prev links.

### E. Hash

Files:
- `labs/e_hash/out/hash_stats.csv`
- `labs/e_hash/out/hash_summary.txt`
- `labs/e_hash/out/avalanche_summary.txt`
- `labs/e_hash/out/hist_61C88647.png`
- `labs/e_hash/out/avalanche_heatmap.png`

Observation:
- `0x61C88647` and `0x9E3779B9` distribute `0..10000` almost identically over 1024 buckets: all buckets used, occupancy range `8..11`, chi-square about `47.63`.
- `0x80000000` collapses the hash into only two buckets, and `0x12345678` uses just 226 buckets, so both are visibly poor constants.
- The Jenkins final-mix avalanche matrix stays close to 50% per cell; the measured mean flip probability is `0.500165`.

Exam takeaway:
- Good multiplicative constants spread high bits well; bad constants create obvious bucket collapse.
- Avalanche quality is about bit-flip diffusion, not just collision count on one dataset.

### F/K. Bits, Overflow, Fixed-Point

Files:
- `labs/fk_bit_fixedpoint/boundary_cases.md`
- `labs/fk_bit_fixedpoint/out/time_after_boundary.csv`
- `labs/fk_bit_fixedpoint/out/ewma_series.csv`
- `labs/fk_bit_fixedpoint/out/pelt_period_contrib.csv`
- `labs/fk_bit_fixedpoint/out/abs_int_min_cases.csv`

Observation:
- `BIT(63)` is valid in this 64-bit setup, `BIT(64)` is not.
- `GENMASK(h, l)` is inclusive, and reversed bounds are invalid.
- `abs(INT_MIN)` is undefined because the positive magnitude does not fit in signed 32-bit.
- `abs_int_min_cases.csv` makes that visible by showing `INT_MIN` is the only sample here whose mathematical absolute value no longer fits in `int32`.
- `time_after()` is reliable only while timestamps differ by less than `2^31`; at exactly `2^31`, both directions report true.
- EWMA warm-up avoids the initial low bias from starting at zero.
- Without period-contrib compensation, `delta=1us` repeated `1e6` times loses all whole periods; with compensation it accumulates to `976` periods and `576us`.

Exam takeaway:
- Half-range arithmetic is the core invariant behind wrap-safe time comparison.
- Fixed-point and decay counters need carry-forward state or small increments disappear.

### C/H. Sort Trace

Files:
- `labs/ch_sort_trace/artifacts/list_sort_trace_n13.txt`
- `labs/ch_sort_trace/artifacts/heapsort_comparisons.csv`

Observation:
- The pending-run lengths for `n=13` evolve as `1`, `2`, `2|1`, `4`, `4|1`, `4|2`, `4|2|1`, `8`, `8|1`, `8|2`, `8|2|1`, `8|4`, `8|4|1`.
- The merge decision follows the binary carry pattern, which is why `trailing_ones` is the useful mental model.
- In this benchmark, bottom-up heapsort uses about 54% to 57% of the comparisons of standard heapsort.

Exam takeaway:
- `list_sort()` pending runs behave like a binary counter over run sizes.
- Bottom-up heap sift-down reduces comparison count substantially even when the algorithmic big-O stays the same.
