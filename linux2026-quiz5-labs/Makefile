SHELL := /bin/sh

LABS := \
	labs/a_bitfield \
	labs/b_ub_alias \
	labs/c_mul_high \
	labs/d_fp16 \
	labs/e_float_quarter

.PHONY: all test clean $(LABS)

all: test

test:
	@set -e; \
	for lab in $(LABS); do \
		if [ -f "$$lab/Makefile" ]; then \
			echo "== $$lab =="; \
			$(MAKE) -C "$$lab" test; \
		else \
			echo "skip $$lab: Makefile not ready"; \
		fi; \
	done

clean:
	@set -e; \
	for lab in $(LABS); do \
		if [ -f "$$lab/Makefile" ]; then \
			$(MAKE) -C "$$lab" clean; \
		fi; \
	done
