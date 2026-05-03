CC ?= cc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2
TARGET := build/bc_list_hlist
SRC := bc_list_hlist.c
ARTIFACTS := artifacts

.PHONY: all build run clean

all: run

build: $(TARGET)

$(TARGET): $(SRC) | build-dir
	$(CC) $(CFLAGS) $< -o $@

build-dir:
	mkdir -p build

run: $(TARGET)
	mkdir -p $(ARTIFACTS)
	./$(TARGET) $(ARTIFACTS)

clean:
	rm -rf build $(ARTIFACTS)
