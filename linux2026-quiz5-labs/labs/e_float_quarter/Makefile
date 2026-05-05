SHELL := /bin/sh

CC ?= cc
CFLAGS ?= -std=c99 -Wall -Wextra -Wpedantic -O2 -g

TARGET := float_quarter_lab

.PHONY: all test demo clean

all: test

$(TARGET): float_quarter_lab.c
	$(CC) $(CFLAGS) $< -o $@

test: $(TARGET)
	./$(TARGET)

demo: $(TARGET)
	./$(TARGET) --demo

clean:
	rm -f $(TARGET)
