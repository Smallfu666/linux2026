CC ?= cc
CFLAGS ?= -O2 -g -std=gnu11
CPPFLAGS ?= -D_POSIX_C_SOURCE=200809L
WARNINGS ?= -Wall -Wextra
LDFLAGS ?=

TARGET := tree_cache_locality

.PHONY: all clean

all: $(TARGET) bench

$(TARGET): src/bench.c src/tree.c src/timing.c src/tree.h src/timing.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) src/bench.c src/tree.c src/timing.c $(LDFLAGS) -o $@

bench: $(TARGET)
	ln -sf $(TARGET) bench

clean:
	rm -f $(TARGET) bench
