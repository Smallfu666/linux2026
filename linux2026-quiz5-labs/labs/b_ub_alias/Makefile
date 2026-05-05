CC ?= gcc
CFLAGS_COMMON ?= -std=c11 -Wall -Wextra -Wpedantic -g
SRC := src/ub_alias_lab.c
BUILD_DIR := build
BIN_O0 := $(BUILD_DIR)/ub_alias.O0
BIN_O2 := $(BUILD_DIR)/ub_alias.O2
BIN_UBSAN := $(BUILD_DIR)/ub_alias.ubsan
ASM_O0 := $(BUILD_DIR)/ub_alias.O0.s
ASM_O2 := $(BUILD_DIR)/ub_alias.O2.s

.PHONY: all run test asm clean

all: $(BIN_O2)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BIN_O0): $(SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS_COMMON) -O0 -o $@ $<

$(BIN_O2): $(SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS_COMMON) -O2 -o $@ $<

$(BIN_UBSAN): $(SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS_COMMON) -O1 -fsanitize=undefined -o $@ $<

$(ASM_O0): $(SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS_COMMON) -O0 -S -o $@ $<

$(ASM_O2): $(SRC) | $(BUILD_DIR)
	$(CC) $(CFLAGS_COMMON) -O2 -S -o $@ $<

asm: $(ASM_O0) $(ASM_O2)

run: $(BIN_O2)
	./$(BIN_O2)

test: $(BIN_O0) $(BIN_O2) $(BIN_UBSAN) asm
	./$(BIN_O0)
	./$(BIN_O2)
	./$(BIN_UBSAN) ub || true

clean:
	rm -rf $(BUILD_DIR)
