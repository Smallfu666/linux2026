CC ?= cc
CSTD ?= gnu11
CFLAGS ?= -O2 -g
WARNFLAGS := -Wall -Wextra

PROGRAMS := container_of_demo pointer_tagging_demo

.PHONY: all run clean

all: $(PROGRAMS)

container_of_demo: container_of_demo.c
	$(CC) $(CFLAGS) -std=$(CSTD) $(WARNFLAGS) $< -o $@

pointer_tagging_demo: pointer_tagging_demo.c
	$(CC) $(CFLAGS) -std=$(CSTD) $(WARNFLAGS) $< -o $@

run: all
	./run.sh

clean:
	rm -f $(PROGRAMS)
	rm -f results/*.txt
