CC ?= cc
CFLAGS ?= -O2 -g
CPPFLAGS ?= -D_POSIX_C_SOURCE=200809L
WARNINGS ?= -Wall -Wextra -Wpedantic
LDFLAGS ?=

BUILD_DIR := build
TARGET := bench
SOURCES := $(wildcard src/*.c)
OBJECTS := $(patsubst src/%.c,$(BUILD_DIR)/%.o,$(SOURCES))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/%.o: src/%.c src/list.h | $(BUILD_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(TARGET)
