SHELL := /bin/sh

CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Wpedantic -O2 -g

TARGET := float32_to_float16_lab

.PHONY: all test clean

all: test

$(TARGET): float32_to_float16_lab.c
	$(CC) $(CFLAGS) $< -o $@

test: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(TARGET)
