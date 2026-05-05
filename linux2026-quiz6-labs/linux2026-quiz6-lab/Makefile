PROGRAMS := fp16_lab bitpack_lab shift_ub_lab ewma_lab tree_cache_lab

GCC ?= gcc
CLANG ?= clang

CSTD := -std=c11
WARN := -Wall -Wextra -Wpedantic
CFLAGS_COMMON := $(CSTD) $(WARN)
UBSAN_FLAGS := -fsanitize=undefined -fsanitize-recover=undefined

GCC_O0_BINS := $(addprefix build/gcc-O0/,$(PROGRAMS))
GCC_O2_BINS := $(addprefix build/gcc-O2/,$(PROGRAMS))
CLANG_O0_BINS := $(addprefix build/clang-O0/,$(PROGRAMS))
CLANG_O2_BINS := $(addprefix build/clang-O2/,$(PROGRAMS))
GCC_UBSAN_BINS := $(addprefix build/gcc-ubsan/,$(PROGRAMS))
CLANG_UBSAN_BINS := $(addprefix build/clang-ubsan/,$(PROGRAMS))

.PHONY: all run sanitize clean dirs perf

all: dirs $(GCC_O0_BINS) $(GCC_O2_BINS) $(CLANG_O0_BINS) $(CLANG_O2_BINS) $(GCC_UBSAN_BINS) $(CLANG_UBSAN_BINS)

dirs:
	mkdir -p build/gcc-O0 build/gcc-O2 build/clang-O0 build/clang-O2 build/gcc-ubsan build/clang-ubsan results

build/gcc-O0/%: src/%.c | dirs
	$(GCC) $(CFLAGS_COMMON) -O0 $< -o $@

build/gcc-O2/%: src/%.c | dirs
	$(GCC) $(CFLAGS_COMMON) -O2 $< -o $@

build/clang-O0/%: src/%.c | dirs
	$(CLANG) $(CFLAGS_COMMON) -O0 $< -o $@

build/clang-O2/%: src/%.c | dirs
	$(CLANG) $(CFLAGS_COMMON) -O2 $< -o $@

build/gcc-ubsan/%: src/%.c | dirs
	$(GCC) $(CFLAGS_COMMON) -O1 $(UBSAN_FLAGS) $< -o $@

build/clang-ubsan/%: src/%.c | dirs
	$(CLANG) $(CFLAGS_COMMON) -O1 $(UBSAN_FLAGS) $< -o $@

run: $(GCC_O2_BINS)
	@for p in $(PROGRAMS); do \
		echo ""; \
		echo "== $$p =="; \
		./build/gcc-O2/$$p; \
	done

sanitize: $(GCC_UBSAN_BINS) $(CLANG_UBSAN_BINS)
	@for cc in gcc-ubsan clang-ubsan; do \
		for p in $(PROGRAMS); do \
			echo ""; \
			echo "== $$cc/$$p =="; \
			./build/$$cc/$$p; \
		done; \
	done

perf: build/gcc-O2/tree_cache_lab
	@if command -v perf >/dev/null 2>&1; then \
		perf stat -e cycles,instructions,cache-references,cache-misses,branches,branch-misses ./build/gcc-O2/tree_cache_lab; \
	else \
		echo "perf not available on this system; skipping perf stat."; \
	fi

clean:
	rm -rf build
	rm -f results/ewma_results.csv
