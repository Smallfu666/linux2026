CC ?= cc
CFLAGS ?= -O2 -std=c11 -Wall -Wextra -pedantic

BIN := ch_sort_trace
ARTIFACT_DIR := artifacts
TRACE_CSV := $(ARTIFACT_DIR)/list_sort_trace_n13.csv
TRACE_TXT := $(ARTIFACT_DIR)/list_sort_trace_n13.txt
BENCH_CSV := $(ARTIFACT_DIR)/heapsort_comparisons.csv

.PHONY: all clean trace bench

all: $(BIN) trace bench

$(BIN): ch_sort_trace.c
	$(CC) $(CFLAGS) $< -o $@

$(ARTIFACT_DIR):
	mkdir -p $@

trace: $(TRACE_TXT)

bench: $(BENCH_CSV)

$(TRACE_CSV): $(BIN) | $(ARTIFACT_DIR)
	./$(BIN) trace --n 13 > $@

$(TRACE_TXT): $(TRACE_CSV)
	tr ',' '\t' < $< > $@

$(BENCH_CSV): $(BIN) | $(ARTIFACT_DIR)
	./$(BIN) bench > $@

clean:
	rm -f $(BIN) $(TRACE_CSV) $(TRACE_TXT) $(BENCH_CSV)
