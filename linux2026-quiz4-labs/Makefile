SHELL := /bin/sh

CC ?= cc
PYTHON ?= python3
CFLAGS_COMMON := -std=c99 -Wall -Wextra -Wpedantic -g
CFLAGS_OPT := -O2
CFLAGS_SAN := -O1 -g -fsanitize=undefined,address -fno-omit-frame-pointer
LDFLAGS_MATH := -lm

RESULTS := results

ALL_TARGETS := fetch dct idct instrument context exp-golomb filter-ablation ub crc color memory report
REPORT_INPUTS := \
	results/source_manifest.txt \
	results/source_sha256.txt \
	results/dct_coef/summary.txt \
	results/idct/summary.txt \
	results/coeff_stats/block_distribution_summary.txt \
	results/coeff_stats/coeff_summary.csv \
	results/compression/compression_metrics.csv \
	results/compression/png_sha256.txt \
	results/context_model/summary.csv \
	results/exp_golomb/exp_golomb_entropy.csv \
	results/filter_ablation/filter_metrics.csv \
	results/ub/hex_uppercase_ub.log \
	results/ub/split_overflow_ub.log \
	results/ub/clamp8_side_effect.log \
	results/ub/decode_unsigned_safe_test.log \
	results/crc/crc_equivalence.txt \
	results/color/status.txt \
	results/memory/size_output.txt \
	results/memory/ulimit_stack.txt \
	results/memory/static_run.log \
	results/memory/stack_run.log \
	results/memory/proc_maps.txt

.PHONY: all $(ALL_TARGETS) clean

all: fetch dct idct instrument context exp-golomb filter-ablation ub crc color memory report

fetch: results/source_manifest.txt results/source_sha256.txt

results/source_manifest.txt results/source_sha256.txt: scripts/fetch_amei.sh
	@mkdir -p results src
	@sh scripts/fetch_amei.sh

dct: results/dct_coef/dct_coef_compare.csv results/dct_coef/summary.txt

results/dct_coef/dct_coef_compare.csv results/dct_coef/summary.txt: src/verify_dct_coef.c src/amei.original.c
	@mkdir -p results/dct_coef
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_OPT) src/verify_dct_coef.c -o results/dct_coef/verify_dct_coef $(LDFLAGS_MATH)
	@./results/dct_coef/verify_dct_coef > /dev/null

idct: results/idct/idct_error.csv results/idct/summary.txt

results/idct/idct_error.csv results/idct/summary.txt: src/compare_idct.c src/amei.original.c
	@mkdir -p results/idct
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_OPT) src/compare_idct.c -o results/idct/compare_idct $(LDFLAGS_MATH)
	@./results/idct/compare_idct > /dev/null

instrument: results/coeff_stats/block_distribution.csv results/coeff_stats/block_distribution_summary.txt results/coeff_stats/coefficients.csv results/coeff_stats/coeff_summary.csv results/compression/amei_instrumented.png results/compression/png_sha256.txt results/compression/compression_metrics.csv

results/compression/amei_instrumented.png results/coeff_stats/block_distribution.csv results/coeff_stats/block_distribution_summary.txt results/coeff_stats/coefficients.csv results/coeff_stats/coeff_summary.csv results/compression/png_sha256.txt results/compression/compression_metrics.csv: src/amei.instrumented.c src/amei.original.c
	@mkdir -p results/compression results/coeff_stats results/filter_ablation
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_OPT) -DAMEI_INSTRUMENT=1 src/amei.instrumented.c -o results/compression/amei_instrumented $(LDFLAGS_MATH)
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_OPT) src/amei.original.c -o results/compression/amei_original $(LDFLAGS_MATH)
	@./results/compression/amei_original > results/compression/amei_original.png
	@./results/compression/amei_instrumented > results/compression/amei_instrumented.png
	@sha256sum results/compression/amei_original.png results/compression/amei_instrumented.png > results/compression/png_sha256.txt
	@./results/compression/amei_instrumented >/dev/null

context: results/context_model/context_adaptation.csv results/context_model/summary.csv

results/context_model/context_adaptation.csv results/context_model/summary.csv: src/simulate_context_model.c
	@mkdir -p results/context_model
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_OPT) src/simulate_context_model.c -o results/context_model/simulate_context_model
	@./results/context_model/simulate_context_model > /dev/null

exp-golomb: results/exp_golomb/exp_golomb_entropy.csv

results/exp_golomb/exp_golomb_entropy.csv: src/exp_golomb_stats.py
	@mkdir -p results/exp_golomb
	@$(PYTHON) src/exp_golomb_stats.py

filter-ablation: results/filter_ablation/amei_none.png results/filter_ablation/amei_deblock_only.png results/filter_ablation/amei_bilateral_only.png results/filter_ablation/amei_full.png results/filter_ablation/filter_metrics.csv

results/filter_ablation/amei_none.png results/filter_ablation/amei_deblock_only.png results/filter_ablation/amei_bilateral_only.png results/filter_ablation/amei_full.png results/filter_ablation/filter_metrics.csv: src/amei.instrumented.c src/amei.original.c scripts/png_metrics.py
	@mkdir -p results/filter_ablation
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_OPT) -DENABLE_DEBLOCK=0 -DENABLE_BILATERAL=0 src/amei.instrumented.c -o results/filter_ablation/amei_none $(LDFLAGS_MATH)
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_OPT) -DENABLE_BILATERAL=0 src/amei.instrumented.c -o results/filter_ablation/amei_deblock_only $(LDFLAGS_MATH)
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_OPT) -DENABLE_DEBLOCK=0 src/amei.instrumented.c -o results/filter_ablation/amei_bilateral_only $(LDFLAGS_MATH)
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_OPT) src/amei.instrumented.c -o results/filter_ablation/amei_full $(LDFLAGS_MATH)
	@./results/filter_ablation/amei_none > results/filter_ablation/amei_none.png
	@./results/filter_ablation/amei_deblock_only > results/filter_ablation/amei_deblock_only.png
	@./results/filter_ablation/amei_bilateral_only > results/filter_ablation/amei_bilateral_only.png
	@./results/filter_ablation/amei_full > results/filter_ablation/amei_full.png
	@$(PYTHON) scripts/png_metrics.py results/filter_ablation/amei_full.png results/filter_ablation/amei_none.png results/filter_ablation/amei_deblock_only.png results/filter_ablation/amei_bilateral_only.png > results/filter_ablation/filter_metrics.csv

ub: results/ub/hex_uppercase_ub.log results/ub/split_overflow_ub.log results/ub/clamp8_side_effect.log results/ub/decode_unsigned_safe_test.log

results/ub/hex_uppercase_ub.log: ub_tests/hex_uppercase_ub.c
	@mkdir -p results/ub
	$(CC) $(CFLAGS_COMMON) -O0 -fsanitize=undefined,address ub_tests/hex_uppercase_ub.c -o results/ub/hex_uppercase_ub
	@ASAN_OPTIONS=detect_leaks=0 ./results/ub/hex_uppercase_ub > results/ub/hex_uppercase_ub.log 2>&1 || true

results/ub/split_overflow_ub.log: ub_tests/split_overflow_ub.c
	@mkdir -p results/ub
	$(CC) $(CFLAGS_COMMON) -O0 -fsanitize=undefined,address ub_tests/split_overflow_ub.c -o results/ub/split_overflow_ub.O0
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_OPT) -fsanitize=undefined,address ub_tests/split_overflow_ub.c -o results/ub/split_overflow_ub.O2
	@echo '=== O0 ===' > results/ub/split_overflow_ub.log
	@ASAN_OPTIONS=detect_leaks=0 ./results/ub/split_overflow_ub.O0 >> results/ub/split_overflow_ub.log 2>&1 || true
	@echo '=== O2 ===' >> results/ub/split_overflow_ub.log
	@ASAN_OPTIONS=detect_leaks=0 ./results/ub/split_overflow_ub.O2 >> results/ub/split_overflow_ub.log 2>&1 || true

results/ub/clamp8_side_effect.log: ub_tests/clamp8_side_effect.c
	@mkdir -p results/ub
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_OPT) ub_tests/clamp8_side_effect.c -o results/ub/clamp8_side_effect
	@./results/ub/clamp8_side_effect > results/ub/clamp8_side_effect.log 2>&1

results/ub/decode_unsigned_safe_test.log: ub_tests/decode_unsigned_safe_test.c
	@mkdir -p results/ub
	$(CC) $(CFLAGS_COMMON) -O0 -fsanitize=undefined,address ub_tests/decode_unsigned_safe_test.c -o results/ub/decode_unsigned_safe_test
	@ASAN_OPTIONS=detect_leaks=0 ./results/ub/decode_unsigned_safe_test > results/ub/decode_unsigned_safe_test.log 2>&1 || true

crc: results/crc/crc_equivalence.txt

results/crc/crc_equivalence.txt: src/crc_nibble_vs_byte.c
	@mkdir -p results/crc
	$(CC) $(CFLAGS_COMMON) $(CFLAGS_OPT) src/crc_nibble_vs_byte.c -o results/crc/crc_nibble_vs_byte
	@./results/crc/crc_nibble_vs_byte > results/crc/crc_equivalence.txt

color: results/color/status.txt

results/color/status.txt: src/color_matrix_analysis.py
	@mkdir -p results/color
	@$(PYTHON) src/color_matrix_analysis.py

memory: results/memory/size_output.txt results/memory/ulimit_stack.txt results/memory/static_run.log results/memory/stack_run.log results/memory/proc_maps.txt

results/memory/size_output.txt results/memory/ulimit_stack.txt results/memory/static_run.log results/memory/stack_run.log results/memory/proc_maps.txt: src/memory_layout_test.c
	@mkdir -p results/memory
	$(CC) $(CFLAGS_COMMON) -O0 src/memory_layout_test.c -o results/memory/memory_layout_test
	@size results/memory/memory_layout_test > results/memory/size_output.txt
	@ulimit -s > results/memory/ulimit_stack.txt
	@./results/memory/memory_layout_test static > results/memory/static_run.log
	@./results/memory/memory_layout_test stack > results/memory/stack_run.log || true
	@./results/memory/memory_layout_test maps > results/memory/proc_maps.txt

report: REPORT.md

REPORT.md: scripts/summarize_results.py $(REPORT_INPUTS)
	@$(PYTHON) scripts/summarize_results.py > REPORT.md

clean:
	rm -rf results REPORT.md
