# linux2026 quiz4 amei.c experiment suite Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a reproducible lab that fetches `amei.c`, runs the requested numeric / entropy / UB / image experiments, and writes evidence to `results/` plus `REPORT.md`.

**Architecture:** Keep `src/amei.original.c` untouched and use a separate `src/amei.instrumented.c` for logging. Implement each standalone probe as a small C or Python program that emits CSV or log files, then drive everything from a single root `Makefile`. Use a pure-stdlib PNG decoder script for metric comparisons so the suite does not depend on Pillow.

**Tech Stack:** POSIX shell, C99, Python 3 stdlib, `cc`, `make`, `sha256sum`, `zlib` via Python stdlib, no third-party Python dependencies.

---

### Task 1: Repository scaffold and source fetch

**Files:**
- Create: `README.md`
- Create: `Makefile`
- Create: `scripts/fetch_amei.sh`
- Create: `scripts/run_all.sh`
- Create: `scripts/summarize_results.py`
- Create: `scripts/png_metrics.py`
- Create: `docs/superpowers/plans/2026-05-04-linux2026-quiz4-amei-lab.md`
- Keep: `src/amei.original.c`

- [ ] **Step 1: Verify the source URL and fetch path**

```sh
sh scripts/fetch_amei.sh
```

- [ ] **Step 2: Check the manifest**

```sh
cat results/source_manifest.txt
cat results/source_sha256.txt
```

- [ ] **Step 3: Verify the source remains unmodified**

```sh
sha256sum src/amei.original.c
```

### Task 2: Numeric validation probes

**Files:**
- Create: `src/verify_dct_coef.c`
- Create: `src/compare_idct.c`

- [ ] **Step 1: Write the DCT recurrence probe and summary writer**
- [ ] **Step 2: Write the IDCT fixed-point vs double reference probe**
- [ ] **Step 3: Build and run `make dct`**
- [ ] **Step 4: Build and run `make idct`**

### Task 3: Decoder instrumentation

**Files:**
- Create: `src/amei.instrumented.c`

- [ ] **Step 1: Add compile-time instrumentation macros and file logging**
- [ ] **Step 2: Log quadtree leaves and coefficient records without changing stdout PNG bytes**
- [ ] **Step 3: Build and run `make instrument`**

### Task 4: Context / entropy / CRC / color / memory probes

**Files:**
- Create: `src/simulate_context_model.c`
- Create: `src/exp_golomb_stats.py`
- Create: `src/crc_nibble_vs_byte.c`
- Create: `src/color_matrix_analysis.py`
- Create: `src/memory_layout_test.c`

- [ ] **Step 1: Implement the arithmetic context adaptation simulator**
- [ ] **Step 2: Implement the Exp-Golomb expectation scan**
- [ ] **Step 3: Implement CRC nibble-vs-byte equivalence**
- [ ] **Step 4: Handle the optional color analysis skip when numpy is missing**
- [ ] **Step 5: Implement the stack/BSS memory layout probe**

### Task 5: UB probes

**Files:**
- Create: `ub_tests/hex_uppercase_ub.c`
- Create: `ub_tests/split_overflow_ub.c`
- Create: `ub_tests/clamp8_side_effect.c`
- Create: `ub_tests/decode_unsigned_safe_test.c`

- [ ] **Step 1: Implement the four minimal UB reproduction programs**
- [ ] **Step 2: Compile each with sanitizer settings and capture logs**
- [ ] **Step 3: Confirm the logs record both sanitizer hits and clean runs**

### Task 6: Reporting

**Files:**
- Create: `REPORT.md`

- [ ] **Step 1: Summarize only observed data, commands, and output files**
- [ ] **Step 2: Generate the final report from the collected artifacts**
- [ ] **Step 3: Run `make report` and inspect the output**
