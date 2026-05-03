CC ?= cc
CFLAGS ?= -O2 -g -std=gnu11
CPPFLAGS ?= -D_POSIX_C_SOURCE=200809L
WARNINGS ?= -Wall -Wextra
LDFLAGS ?=

COMMON_SOURCES := src/bytecode.c src/interpreter.c src/timing.c
COMMON_HEADERS := src/bytecode.h src/interpreter.h src/timing.h
TARGETS := dispatch_benchmark bench_switch bench_cgoto

.PHONY: all clean

all: $(TARGETS)

dispatch_benchmark: $(COMMON_SOURCES) src/bench.c $(COMMON_HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) $(COMMON_SOURCES) src/bench.c $(LDFLAGS) -o $@

bench_switch: $(COMMON_SOURCES) src/bench_mode_common.c src/bench_switch.c $(COMMON_HEADERS) src/bench_mode_common.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) $(COMMON_SOURCES) src/bench_mode_common.c src/bench_switch.c $(LDFLAGS) -o $@

bench_cgoto: $(COMMON_SOURCES) src/bench_mode_common.c src/bench_cgoto.c $(COMMON_HEADERS) src/bench_mode_common.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) $(COMMON_SOURCES) src/bench_mode_common.c src/bench_cgoto.c $(LDFLAGS) -o $@

clean:
	rm -f $(TARGETS)
