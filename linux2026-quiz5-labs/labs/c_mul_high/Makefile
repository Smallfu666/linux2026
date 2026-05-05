CC ?= cc
CFLAGS ?= -std=gnu11 -O2 -Wall -Wextra
CPPFLAGS ?=
LDFLAGS ?=

TARGET := build/c_mul_high_lab
SRC := src/c_mul_high_lab.c

.PHONY: all test run clean

all: $(TARGET)

$(TARGET): $(SRC)
	@mkdir -p build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(SRC) -o $@ $(LDFLAGS)

test: $(TARGET)
	./$(TARGET)

run: test

clean:
	rm -rf build
