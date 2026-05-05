CC ?= gcc
CFLAGS ?= -std=c11 -Wall -Wextra -Wpedantic -O2 -g
SRC := src/bitfield_lab.c
BUILD_DIR := build
TARGET := $(BUILD_DIR)/bitfield_lab

.PHONY: all run test clean

all: $(TARGET)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(TARGET): $(SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS) -o $@ $<

run: $(TARGET)
	./$(TARGET)

test: $(TARGET)
	./$(TARGET)

clean:
	rm -rf $(BUILD_DIR)
