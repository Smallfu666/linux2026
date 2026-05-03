CC ?= cc
CFLAGS ?= -O2 -g
CPPFLAGS ?= -D_POSIX_C_SOURCE=200809L
WARNINGS ?= -Wall -Wextra -Wpedantic
LDFLAGS ?=

TARGETS := ieee754_decode fp_assoc

.PHONY: all clean

all: $(TARGETS)

ieee754_decode: ieee754_decode.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) $< $(LDFLAGS) -o $@

fp_assoc: fp_assoc.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) $< $(LDFLAGS) -o $@

clean:
	rm -f $(TARGETS)
