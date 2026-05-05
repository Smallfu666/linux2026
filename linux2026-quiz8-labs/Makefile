CC ?= gcc
PYTHON ?= python3

ROOT := $(CURDIR)
BUILD := $(ROOT)/build
RESULTS := $(ROOT)/results
SRC := $(ROOT)/src
SCRIPTS := $(ROOT)/scripts

COMMON_CFLAGS := -std=c11 -Wall -Wextra -Wpedantic -O2 -g -I$(SRC)
COMMON_LDFLAGS :=

BLOOM_BIN := $(BUILD)/bloom_filter
SIDE_BIN := $(BUILD)/side_channel
MODULO_BIN := $(BUILD)/modulo_bias
SCHED_BIN := $(BUILD)/scheduler

.PHONY: all test lint dev clean run-bloom run-side-channel run-modulo run-scheduler-unsafe run-scheduler-safe

all: $(BLOOM_BIN) $(SIDE_BIN) $(MODULO_BIN) $(SCHED_BIN)

$(BUILD):
	mkdir -p $(BUILD) $(RESULTS)

$(BLOOM_BIN): $(BUILD) src/bloom_filter/main.c src/bloom_filter/hash.c | $(BUILD)
	$(CC) $(COMMON_CFLAGS) src/bloom_filter/main.c src/bloom_filter/hash.c -o $@ $(COMMON_LDFLAGS)

$(SIDE_BIN): $(BUILD) src/side_channel/main.c src/side_channel/crc.c | $(BUILD)
	$(CC) $(COMMON_CFLAGS) -O0 src/side_channel/main.c src/side_channel/crc.c -o $@ $(COMMON_LDFLAGS)

$(MODULO_BIN): $(BUILD) src/modulo_bias/main.c | $(BUILD)
	$(CC) $(COMMON_CFLAGS) src/modulo_bias/main.c -o $@ $(COMMON_LDFLAGS)

$(SCHED_BIN): $(BUILD) src/scheduler/main.c | $(BUILD)
	$(CC) $(COMMON_CFLAGS) src/scheduler/main.c -o $@ $(COMMON_LDFLAGS)

run-bloom: $(BLOOM_BIN)
	$(BLOOM_BIN)

run-side-channel: $(SIDE_BIN)
	$(SIDE_BIN)

run-modulo: $(MODULO_BIN)
	$(MODULO_BIN) $(RESULTS)/modulo_bias.csv 100000000 1009

run-scheduler-unsafe: $(SCHED_BIN)
	$(SCHED_BIN) unsafe

run-scheduler-safe: $(SCHED_BIN)
	$(SCHED_BIN) safe

test: all
	$(BLOOM_BIN) >/dev/null
	$(SIDE_BIN) >/dev/null
	$(MODULO_BIN) $(RESULTS)/modulo_bias.smoke.csv 100000 1009 >/dev/null
	$(PYTHON) $(SCRIPTS)/plot_bias.py $(RESULTS)/modulo_bias.smoke.csv $(RESULTS)/modulo_bias.smoke.svg
	sh -c 'timeout 2s $(SCHED_BIN) safe >/dev/null; status=$$?; [ $$status -eq 0 ] || [ $$status -eq 124 ]'

lint:
	$(CC) $(COMMON_CFLAGS) -fsyntax-only src/bloom_filter/main.c src/bloom_filter/hash.c
	$(CC) $(COMMON_CFLAGS) -fsyntax-only -O0 src/side_channel/main.c src/side_channel/crc.c
	$(CC) $(COMMON_CFLAGS) -fsyntax-only src/modulo_bias/main.c
	$(CC) $(COMMON_CFLAGS) -fsyntax-only src/scheduler/main.c
	$(PYTHON) -m py_compile $(SCRIPTS)/plot_bias.py

dev: all
	@echo "Targets ready: run-bloom run-side-channel run-modulo run-scheduler-unsafe run-scheduler-safe"

clean:
	rm -rf $(BUILD) $(RESULTS)
